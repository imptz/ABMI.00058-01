SECTIONS
{
	.text :	
	{
		code = .;
		 *(.text)
		start_ctors = .;
		*(.ctors)
		end_ctors = .;
		
		start_dtors = .;
		*(.dtors)
		end_dtors = .;
		
		KEEP(SORT(*)(.ctors))
		KEEP(SORT(*)(.dtors))
	}

	.rodata : 
	{
		*(.rodata)
	}

	.data : 
	{
		data = .;
		*(.data)
   	}

	.bss : 
	{
		bss = .;
		*(.bss)
	}

	/DISCARD/ :
	{
		*(.comment)
		*(.comment*)
	}
	
	end = .;
}
