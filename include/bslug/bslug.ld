SECTIONS {
	.bslug.meta : {
		*(.bslug.meta*)
	}
	.bslug.load : {
		*(.bslug.load*)
	}
    .bslug.data : {
        *(.bslug.data*);
    }
	.text : {
		*(.text*)
	}
	.data : {
		*(.data*)
	}
	.rodata : {
		*(.rodata*)
	}
	.bss : {
		*(.bss*)
	}
	/DISCARD/ : {
		*(*)
	}
}