#include <redmule.hpp>

#include <stdio.h>

enum buffers {W_BUF, X_BUF, Y_BUF, Z_BUF, SKIP};

void RedMule::reset_sched() {
    this->preload_cnt       = 0;
	this->compute_cnt       = 0;
	this->store_ctn         = 0;
	this->hypercycle_cnt    = 0;
	this->cycle_cnt         = 0;
    this->subcycle_cnt      = 0;
    this->w_cols_iters      = 0;
    this->x_rows_iters      = 0;
    this->z_cycle_stores    = 0;
    this->z_strb            = -1;
    this->last_x_row        = false;
    this->done              = false;
}

bool RedMule::preload_iter(int* latency) {
    int ITER_TOTAL = ARRAY_WIDTH * ARRAY_HEIGHT * (PIPE_REGS + 1) / this->nb_tcdm_banks;

    if (this->preload_cnt < ITER_TOTAL) {
        *latency = this->buf_disamb(Y_BUF, -1);
    } else {
        *latency = this->buf_disamb(X_BUF, -1);
    }

    this->preload_cnt++;

    if (this->preload_cnt == 3 * ITER_TOTAL) {
        this->preload_cnt = 0;

        return true;
    } else {
        return false;
    }
}

int RedMule::buf_disamb(int label, strobe_t strb) {
    int latency = 0;
    
    switch (label) {
        case W_BUF:
            latency = this->w_stream.iterate(this->buffers.get_next_w(), strb);
            break;

        case X_BUF:
            latency = this->x_stream.iterate(this->buffers.get_next_x(), strb);
            break;

        case Y_BUF:
            latency = this->y_stream.iterate(this->buffers.get_next_y(), strb);
            break;

        case Z_BUF:
            if (this->last_x_row && this->z_cycle_stores > ((this->register_file [REDMULE_REG_LEFTOVERS_PTR>>2] >> 24) & 0x000000ff)) {
                latency = this->z_stream.iterate(this->buffers.get_next_z(), 0);
            } else {
                latency = this->z_stream.iterate(this->buffers.get_next_z(), this->z_strb);
            }

            this->z_cycle_stores++;
            break;

        case SKIP:
            latency = 1;
            break;

        default:
            this->trace.fatal("buf_disamb: UNKNOWN LABEL (%d)\n", label);
            latency = 1;
            
    }

    return latency;
}

int RedMule::subcycle_routine(bool skip_w, int label, strobe_t strb) {
    int latency = 0;

    int cycle_mod = this->subcycle_cnt % (PIPE_REGS + 1);

    if (cycle_mod == 0) {
        latency = skip_w ? this->buf_disamb(SKIP, strb) : this->buf_disamb(W_BUF, strb);
    } else if (cycle_mod < ARRAY_WIDTH / ARRAY_HEIGHT + 1) {
        latency = this->buf_disamb(label, strb);
    } else {
        latency = this->buf_disamb(SKIP, strb);
    }

    this->subcycle_cnt++;

    if (this->subcycle_cnt == ARRAY_HEIGHT * (PIPE_REGS + 1)) {
        this->subcycle_cnt = 0;
        this->cycle_cnt++;

        this->z_cycle_stores = 0;
    }

    // this->trace.msg("Subcycle latency: %d \n", latency);

    return latency;
}

void RedMule::first_iter_routine(int* latency) {
    switch (cycle_cnt) {
        case 1:
            *latency = this->subcycle_routine(false, Y_BUF, -1);
            break;

        case PIPE_REGS - 1:
            *latency = this->subcycle_routine(false, X_BUF, -1);
            break;

        default:
            *latency = this->subcycle_routine(false, SKIP, -1);
    }
}

void RedMule::standard_iter_routine(int* latency) {
    switch (cycle_cnt) {
        case PIPE_REGS - 1:
            *latency = this->subcycle_routine(false, X_BUF, -1);
            break;

        default:
            *latency = this->subcycle_routine(false, SKIP, -1);
    }
}

void RedMule::last_iter_routine(int* latency) {
    return;
}

bool RedMule::compute_iter(int* latency) {
    if (this->hypercycle_cnt == 0) {
        this->first_iter_routine(latency);
    } else {
        this->standard_iter_routine(latency);
    }

    if (this->cycle_cnt == (PIPE_REGS + 1)) {
        this->cycle_cnt = 0;
        this->hypercycle_cnt++;
    }

    // this->trace.msg("this->cycle_cnt: %d, this->subcycle_cnt: %d, this->hypercycle_cnt: %d, Require: %d \n", \
    //     this->cycle_cnt, this->subcycle_cnt, this->hypercycle_cnt, (this->register_file [REDMULE_REG_X_D1_STRIDE_PTR>>2] / sizeof(src_fmt_t) / ARRAY_WIDTH - 1));

    if (this->hypercycle_cnt == (this->register_file [REDMULE_REG_X_ITER_PTR>>2] & 0x0000ffff)) {
        this->buffers.compute_z();
        if (this->cycle_cnt != 0 || this->subcycle_cnt != 0)
        {
            this->trace.fatal("this->cycle_cnt: %d, this->subcycle_cnt: %d \n", this->cycle_cnt, this->subcycle_cnt);
        }
        return true;
    }

    return false;
}

bool RedMule::store_iter(int* latency) {
    if (this->cycle_cnt == 0 && this->subcycle_cnt == 0) {
        this->w_cols_iters++;
        this->trace.msg("w_cols_iters: %d, x_rows_iters: %d, hyper_require: %d \n", w_cols_iters, x_rows_iters, (this->register_file [REDMULE_REG_X_ITER_PTR>>2] & 0x0000ffff));

        if (this->w_cols_iters == (this->register_file [REDMULE_REG_W_ITER_PTR>>2] & 0x0000ffff)) {
            this->w_cols_iters = 0;
            this->x_rows_iters++;

            if ((this->register_file [REDMULE_REG_LEFTOVERS_PTR>>2] & 0x000000ff) != 0) {
                this->z_strb = 0;
                
                uint64_t msk = 2 * sizeof(dst_fmt_t) - 1;

                switch (sizeof(dst_fmt_t)) {
                    case 1:
                        msk = 0x1;
                        break;

                    case 2:
                        msk = 0x3;
                        break;

                    case 4:
                        msk = 0xF;
                        break;
                }

                for (int i = 0; i < (this->register_file [REDMULE_REG_LEFTOVERS_PTR>>2] & 0x000000ff); i++) {
                    this->z_strb = this->z_strb | msk;

                    msk = msk << sizeof(dst_fmt_t);
                }
            }

            if (this->x_rows_iters == (this->register_file [REDMULE_REG_X_ITER_PTR >> 2]) >> 16) {
                this->done = true;

                if (((this->register_file [REDMULE_REG_LEFTOVERS_PTR>>2] >> 24) & 0x000000ff) != 0) {
                    this->last_x_row = true;
                }
            }
        }
    }


    // switch (cycle_cnt) {
    //     case 0:
    //         *latency = this->subcycle_routine(false, Z_BUF, -1);
    //         break;

    //     case 1:
    //         *latency = this->subcycle_routine(false, Y_BUF, -1);
    //         break;

    //     case ARRAY_HEIGHT - 1:
    //         *latency = this->subcycle_routine(false, X_BUF, -1);
    //         break;

    //     default:
    //         *latency = this->subcycle_routine(false, SKIP, -1);
    // }
    *latency = 1;
    this->cycle_cnt++;

    if (this->cycle_cnt == (ARRAY_WIDTH * ARRAY_HEIGHT * (PIPE_REGS + 1) * sizeof(dst_fmt_t) / (this->nb_tcdm_banks * 4))) {
        this->cycle_cnt = 0;
        this->hypercycle_cnt = 0;
        this->subcycle_cnt = 0;

        this->z_strb = -1;
        this->last_x_row = false;

        return true;
    }

    return false;
}