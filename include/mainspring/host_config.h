#define HOST_LINUX 1
#define HOST_BITS32 0
#define HOST_BITS64 1
#define HOST_BITS128 0

#if (HOST_BITS64)
	typedef unsigned long	native;
	typedef unsigned long	qword;
	typedef unsigned int	dword;
	typedef unsigned short	word;
	typedef unsigned char	byte;
#endif
