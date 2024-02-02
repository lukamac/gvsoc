import gvsoc.systree as st

class RedMule(st.Component):

    def __init__(self, parent, name, nb_tcdm_banks):

        super(RedMule, self).__init__(parent, name)

        self.set_component('pulp.redmule.redmule')

        self.add_properties({
            'nb_tcdm_banks': nb_tcdm_banks,
        })
