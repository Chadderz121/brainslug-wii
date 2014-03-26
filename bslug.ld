SECTIONS {
	.bslug.meta : {
		*(.bslug.meta*)
	}
	.bslug.load : {
		*(.bslug.load*)
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