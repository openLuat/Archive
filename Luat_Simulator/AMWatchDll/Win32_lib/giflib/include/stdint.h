#ifndef __STDINT_H__
#define __STDINT_H__

/* Signed.  */
typedef signed char		int8_t;
typedef short int		int16_t;
typedef int			int32_t;
#if defined(_WIN32) && defined(_WIN64)
	typedef long int		int64_t;
#else
	typedef long long int	int64_t;
#endif //_WIN32 && _WIN64

/* Unsigned.  */
typedef unsigned char		uint8_t;
typedef unsigned short int	uint16_t;
typedef unsigned int		uint32_t;
#if defined(_WIN32) && defined(_WIN64)
	typedef unsigned long int	uint64_t;
#else
	typedef unsigned long long int	uint64_t;
#endif //_WIN32 && _WIN64

#endif //__STDINT_H__