SECTIONS {
	.bslug.meta : {
        bslug_meta = .;
		*(.bslug.meta*)
	}
	.bslug.load : {
        bslug_load = .;
		*(.bslug.load*)
	}
}