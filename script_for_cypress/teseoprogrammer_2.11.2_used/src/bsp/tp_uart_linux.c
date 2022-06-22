/*******************************************************************************
*                            (C) 2009 STMicroelectronics
*    Reproduction and Communication of this document is strictly prohibited
*      unless specifically authorized in writing by STMicroelectronics.
*-----------------------------------------------------------------------------
*                                  APG / CRM / SA&PD
*                   Software Development Group - SW platform & HW Specific
******************************************************************************/

/*****************************************************************************
includes
*****************************************************************************/

#include "tp_common.h"
#include "tp_uart.h"
#include "tp_tools.h"
#include "tp_print.h"

//#include <windows.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
/*****************************************************************************
external declarations
*****************************************************************************/

/*****************************************************************************
defines and macros (scope: module-local)
*****************************************************************************/

#define UART_STANDARD(x)   TP_PRINT(x)     
#define UART_ERROR(x)      TP_ERROR(x)  
#define UART_DEBUG(x)      TP_INFO(x) 
#define UART_STREAM(x)     TP_STREAM(x)

#define TP_UART_FILENAME_PREFIX     "\\\\.\\"

#define TP_UART_BAUDRATE_TABLESIZE  5U

#define TP_UART_READBUFFERSIZE      32U
#define INVALID_HANDLE_VALUE        -1
/*****************************************************************************
typedefs and structures (scope: module-local)
*****************************************************************************/
typedef int HANDLE;
typedef struct tp_uart_manager_s
{
	tPChar    comname;
	HANDLE    handle;
	tU32      buf_pos;
	tPChar    readbuffer;
} tp_uart_manager_t;

/*****************************************************************************
global variable definitions  (scope: module-exported)
*****************************************************************************/

/*****************************************************************************
global variable definitions (scope: module-local)
*****************************************************************************/

#ifdef MULTI_COMS
pthread_mutex_t mutex_uart = PTHREAD_MUTEX_INITIALIZER;
#endif

static const tUInt tp_uart_baudrate_table[TP_UART_BAUDRATE_TABLESIZE] =
{
	57600,
	115200,
	230400,
	460800,
	921600,
};

static tp_uart_manager_t * tp_uart_manager[TP_COM_INSTANCE_MAX];

/*****************************************************************************
function prototypes (scope: module-local)
*****************************************************************************/

/*****************************************************************************
function implementations (scope: module-local)
*****************************************************************************/

/*****************************************************************************
function implementations (scope: module-exported)
*****************************************************************************/


static int usb_init(char* usb_set)
{
	int iFd = -1;
	struct termios opt;
	// iFd = open(usb_set, O_RDWR | O_NOCTTY);  //TODO:
	iFd = open(usb_set, O_RDWR | O_NOCTTY | O_NDELAY);
    if(iFd < 0) {
        perror(usb_set);
        return -1;
    }

    tcgetattr(iFd, &opt);

    cfsetispeed(&opt, B115200);
    cfsetospeed(&opt, B115200);


    /*
     * raw mode
     */
    opt.c_lflag   &=   ~(ECHO   |   ICANON   |   IEXTEN   |   ISIG);
    opt.c_iflag   &=   ~(BRKINT   |   ICRNL   |   INPCK   |   ISTRIP   |   IXON);
    opt.c_oflag   &=   ~(OPOST);
    opt.c_cflag   &=   ~(CSIZE   |   PARENB);
    opt.c_cflag   |=   CS8;

	// fcntl(iFd,F_SETFL,0);            //TODO:
    // logg(MESSAGE,"O_NONBLOCK = %d\r\n",O_NONBLOCK);
    fcntl(iFd,F_SETFL,O_NONBLOCK);
    /*
     * 'DATA_LEN' bytes can be read by serial
     */
    opt.c_cc[VMIN]   =   0;
    opt.c_cc[VTIME]  =   0;

    if (tcsetattr(iFd,   TCSANOW,   &opt)<0) {
        return   -1;
    }
    return iFd;
}


// 'ABC' => 'abc'
int Upper2Lower(void* sBuff, int slen)
{
	char	*bsBuff = (char *)sBuff;
	for (int i=0;i<slen;i++)
    {
        if((bsBuff[i]>='A') && (bsBuff[i]<='Z') && (bsBuff[i]!='U') && (bsBuff[i]!='S') && (bsBuff[i]!='B'))
        {
            bsBuff[i] |= 0x20;
        }
    }
	return true;
}


tp_status_t tp_uart_init(tp_prgdev_uart_config_t *uart_cfg, tp_prgdev_instance_t inst)
{
#if 1
	tp_status_t error = TP_STATUS_OK;
    dbg_printf("test1\r\n");
	LOCK_UART

		if (inst >= TP_COM_INSTANCE_MAX)
		{
			UART_ERROR(("Too many port com" TP_ENDL));
			error = TP_STATUS_ERROR;
		}

	tp_uart_manager[inst] = (tp_uart_manager_t*)_clibs_calloc(TP_COM_INSTANCE_MAX, sizeof(tp_uart_manager_t));
    dbg_printf("test100\r\n");
	if (tp_uart_manager == NULL || tp_uart_manager[inst] == NULL)
	{
		UART_ERROR(("Not enough memory!" TP_ENDL));
		error = TP_STATUS_ERROR;
	}
    dbg_printf("test101\r\n");
	/* Create UART filename */
	if (TP_STATUS_OK == error)
	{
		tUInt filename_len = 1 + _clibs_strlen(TP_UART_FILENAME_PREFIX) + _clibs_strlen(uart_cfg->port[inst]);

		tp_uart_manager[inst]->comname = (tPChar)_clibs_malloc(filename_len);
		tp_uart_manager[inst]->readbuffer = (tPChar)_clibs_malloc(TP_UART_READBUFFERSIZE);

		if (NULL == tp_uart_manager[inst]->comname || tp_uart_manager[inst]->readbuffer == NULL)
		{
			UART_ERROR(("Not enough memory!" TP_ENDL));
			error = TP_STATUS_ERROR;
		}
		else
		{
			//_clibs_strcpy(tp_uart_manager[inst]->comname, TP_UART_FILENAME_PREFIX);
			//_clibs_strcpy(&tp_uart_manager[inst]->comname[_clibs_strlen(TP_UART_FILENAME_PREFIX)], uart_cfg->port[inst]);
            dbg_printf("test1-1");
            Upper2Lower(uart_cfg->port[inst],strlen(uart_cfg->port[inst]));
            _clibs_strcpy(tp_uart_manager[inst]->comname,uart_cfg->port[inst]);
            tp_uart_manager[inst]->comname[filename_len - 1] = '\0';
		}
	}

	/* Try opening UART COM to report any error */
	if (TP_STATUS_OK == error)
	{
        #if 0
		tp_uart_manager[inst]->handle = CreateFileA(
			tp_uart_manager[inst]->comname,
			GENERIC_READ | GENERIC_WRITE,
			0,                          /* no share  */
			NULL,                       /* no security */
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,      /* no threads */
			NULL                        /* no templates */
		);
        #endif
        dbg_printf("comname = %s\r\n",tp_uart_manager[inst]->comname);
        tp_uart_manager[inst]->handle = usb_init(tp_uart_manager[inst]->comname);
        // tp_uart_flowcontrol(1,1,100,inst);
        dbg_printf("tp_uart_manager[%d]->handle = %d\r\n",inst,tp_uart_manager[inst]->handle);
		if (INVALID_HANDLE_VALUE == tp_uart_manager[inst]->handle)
		{
			UART_ERROR(("Uart %s not found, instance = %d" TP_ENDL, uart_cfg->port[inst], inst));
			error = TP_STATUS_ERROR;
		}
		else
		{
			UART_DEBUG(("%s opened, instance = %d" TP_ENDL, uart_cfg->port[inst], inst));
		}
	}
    dbg_printf("2close\r\n");
	tp_uart_close(inst);
	UNLOCK_UART
	return error;
#endif
}
// extern tp_manager_t   tp_manager;
tp_status_t tp_uart_open(tp_prgdev_instance_t inst)
{
#if 1
	tp_status_t error = TP_STATUS_OK;
	LOCK_UART

		if (-1 == tp_uart_manager[inst]->handle)
		{
            tp_uart_manager[inst]->handle = usb_init(tp_uart_manager[inst]->comname);
       
            if (INVALID_HANDLE_VALUE == tp_uart_manager[inst]->handle)
            {
				UART_ERROR(("Uart %s not found!" TP_ENDL, tp_uart_manager[inst]->comname));
                error = TP_STATUS_ERROR;
            }
            else
            {
				UART_DEBUG(("%s opened, instance = %d" TP_ENDL, tp_uart_manager[inst]->comname, inst));
            }
		}
		else
		{
			UART_ERROR(("Uart instance %d not found!" TP_ENDL, inst));
            dbg_printf("have open----------------------------------------\r\n");
		}

	    UNLOCK_UART
		return error;
#endif
}

tp_status_t tp_uart_close(tp_prgdev_instance_t inst)
{
    tp_status_t error = TP_STATUS_OK;

	if (tp_uart_manager[inst] == NULL)
		return error;
	if (INVALID_HANDLE_VALUE != tp_uart_manager[inst]->handle)
	{
        close(tp_uart_manager[inst]->handle);
		UART_DEBUG(("Close uart, instance = %d" TP_ENDL, inst));
	}
	else
	{
		UART_ERROR(("Close uart failure, instance = %d" TP_ENDL, inst));
	}

    dbg_printf("----------------------------- close inst = %d\r\n",inst);
	tp_uart_manager[inst]->handle = INVALID_HANDLE_VALUE;
	return error;
}

tp_status_t tp_uart_flowcontrol(boolean rts, boolean dtr, tUInt timing, tp_prgdev_instance_t inst)
{
    int nSpeed = 230400;
    int nBits = 8;
    char nEvent = 'N';
    int nStop = 1;
	struct termios newtio,oldtio;
	if  ( tcgetattr(tp_uart_manager[inst]->handle,&oldtio)  !=  0) {
		perror("SetupSerial mcu");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch( nBits )
	{
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
	}

	switch( nEvent )
	{
		case 'O':
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			newtio.c_iflag |= (INPCK | ISTRIP);
			break;
		case 'E':
			newtio.c_iflag |= (INPCK | ISTRIP);
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			break;
		case 'N':
			newtio.c_cflag &= ~PARENB;
			break;
	}
#if 0
	switch( nSpeed )
	{
		case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
		case 4800:
			cfsetispeed(&newtio, B4800);
			cfsetospeed(&newtio, B4800);
			break;
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 57600:
			cfsetispeed(&newtio, B57600);
			cfsetospeed(&newtio, B57600);
			break;
		  case 19200:
			cfsetispeed(&newtio, B19200);
			cfsetospeed(&newtio, B19200);
			break;
		case 38400:
			cfsetispeed(&newtio, B38400);
			cfsetospeed(&newtio, B38400);
			break;
		case 230400:
			cfsetispeed(&newtio, B460800);
			cfsetospeed(&newtio, B460800);
			break;
		case 460800:
			cfsetispeed(&newtio, B460800);
			cfsetospeed(&newtio, B460800);
			break;
		default:
			cfsetispeed(&newtio, B38400);
			cfsetospeed(&newtio, B38400);
			break;
	}
#endif
	if( nStop == 1 )
		newtio.c_cflag &=  ~CSTOPB;
	else if ( nStop == 2 )
	newtio.c_cflag |=  CSTOPB;
	newtio.c_cc[VTIME]  = 150;
	newtio.c_cc[VMIN] = 255;
	tcflush(tp_uart_manager[inst]->handle,TCIFLUSH);
	if((tcsetattr(tp_uart_manager[inst]->handle,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}
	//prC8f("set done!\n");
	return 0;
}

tSize tp_uart_read(tPChar out_buf, const tSize chars, const tUInt timeout, tp_prgdev_instance_t inst)
{
#if 1
	tUInt read_chars = 0U;
	tUInt chars_to_read = 0U;

	// COMMTIMEOUTS uart_timeouts = { 0 };

	/* Get chars from buffer */
	if (tp_uart_manager[inst]->buf_pos >= chars)
	{
		/* The read buffer already contains enough chars */
		memcpy(out_buf, tp_uart_manager[inst]->readbuffer, chars);
		tp_uart_manager[inst]->buf_pos -= chars;
        dbg_printf("chars = %d\r\n",chars);
		return chars;
	}
	else
	{
		/* Copy only available chars */
		chars_to_read = chars - tp_uart_manager[inst]->buf_pos;
        fd_set rfds, oldfds;
        struct timeval tv;
        tv.tv_sec = 0;  
        tv.tv_usec = timeout * 1000;
        FD_ZERO(&oldfds);
		FD_SET(tp_uart_manager[inst]->handle, &oldfds);
        int ret = select(tp_uart_manager[inst]->handle + 1, &oldfds, NULL, NULL, &tv);
        if(ret > 0)
        {
			if(!FD_ISSET(tp_uart_manager[inst]->handle,&oldfds))  
				{dbg_printf("======================================\r\n");};  
		/* Prepare UART */
		// uart_timeouts.ReadTotalTimeoutConstant = timeout;
		// SetCommTimeouts(tp_uart_manager[inst]->handle, &uart_timeouts);
            read_chars = read(tp_uart_manager[inst]->handle, &tp_uart_manager[inst]->readbuffer[tp_uart_manager[inst]->buf_pos], chars_to_read);
        }
        //ReadFile(tp_uart_manager[inst]->handle, &tp_uart_manager[inst]->readbuffer[tp_uart_manager[inst]->buf_pos], chars_to_read, (LPDWORD)(&read_chars), NULL);
    	dbg_printf("read_chars = %d,chars_to_read = %d,handle[%d] = %d\r\n",read_chars,chars_to_read,inst,tp_uart_manager[inst]->handle);
		if (read_chars != 0)
			UART_STREAM(("Read %d chars from %s instance = %d" TP_ENDL, read_chars, tp_uart_manager[inst]->comname, inst));

		if (0U < read_chars)
		{
			if (read_chars == chars_to_read)
			{
				/* All wanted chars has been read. Reset read buffer */
				memcpy(out_buf, tp_uart_manager[inst]->readbuffer, chars);
				tp_uart_manager[inst]->buf_pos = 0U;
			}
			else
			{
				/* Add chars to read buffer for next read */
				tp_uart_manager[inst]->buf_pos += read_chars;
			}
		}
		else
		{
			/* No more chars read, keep read buffer */
		}

		return read_chars;
	}
#endif
}

tSize tp_uart_write(const tPChar in_buf, const tSize chars, tp_prgdev_instance_t inst)
{
	tInt written_chars = 0;
	// DWORD last_error;
	// BOOL write_status;


	if (tp_uart_manager[inst]->handle == NULL)
	{
		UART_ERROR(("Handle is not valid, instance = %d" TP_ENDL, inst));
		return 0;
	}
    // written_chars = write(tp_uart_manager[inst]->handle, in_buf ,chars);

    int bytes_to_send = 0;
    #define MAX_TO_SEND   2000
    int  src_ptr = 0;
    tUInt remaining_bytes = chars;
    dbg_printf("remaining_bytes = %d\r\n",remaining_bytes);
	while (0 < remaining_bytes)
	{
		// /* Check bytes to sent */
		// if (MAX_TO_SEND < remaining_bytes)
		// {
		// 	bytes_to_send = MAX_TO_SEND;
		// 	remaining_bytes -= MAX_TO_SEND;
        //     usleep(100);
		// }
		// else
		// {
		// 	bytes_to_send = remaining_bytes;
		// 	remaining_bytes = 0;
        //     usleep(100);
		// }
		/* Send buffer */
        written_chars = write(tp_uart_manager[inst]->handle, in_buf+src_ptr ,remaining_bytes);
        if(written_chars >= 0)
        {
            remaining_bytes-= written_chars;
		    src_ptr += written_chars;
        	dbg_printf("src_ptr = %d,written_chars = %d,remaining_bytes = %d\r\n",src_ptr,written_chars,remaining_bytes);

        }
        else
        {
            dbg_printf("++++++++++++++++++++written_chars = %d\r\n",written_chars);
        }
        

	}
#if 0
	while (0 < remaining_bytes)
	{
		/* Check bytes to sent */
		if (MAX_TO_SEND < remaining_bytes)
		{
			bytes_to_send = MAX_TO_SEND;
			remaining_bytes -= MAX_TO_SEND;
            usleep(100);
		}
		else
		{
			bytes_to_send = remaining_bytes;
			remaining_bytes = 0;
            usleep(100);
		}
		/* Send buffer */
        written_chars = write(tp_uart_manager[inst]->handle, in_buf+src_ptr ,bytes_to_send);
        dbg_printf("bytes_to_send = %d,written_chars = %d,remaining_bytes = %d\r\n",bytes_to_send,written_chars,inst,remaining_bytes);
		src_ptr += bytes_to_send;
	}
#endif
    // for(int i=0;i<chars;i++)
    // {
    //     dbg_printf("%d  \r\n",in_buf[i]);
    // }
    dbg_printf("\r\n");

	return written_chars;
}

tBool tp_uart_getbaudrateidx(const tUInt baudrate, tPU8 idx_ptr)
{
#if 1
	tBool valid = FALSE;
	tUInt cnt = 0;

	while ((TP_UART_BAUDRATE_TABLESIZE > cnt) && (FALSE == valid))
	{
		if (baudrate == tp_uart_baudrate_table[cnt])
		{
			if (NULL != idx_ptr)
			{
				*idx_ptr = cnt;
			}
			valid = TRUE;
		}
		else
		{
			cnt++;
		}
	}

	UART_DEBUG(("Baud rate index translation %d -> %d" TP_ENDL, baudrate, *idx_ptr));

	return valid;
#endif
}


//������ʲô����???
tp_status_t tp_uart_setbaudrate(const tUInt baudrate, tp_prgdev_instance_t inst)
{
#if 0
	tp_status_t error = TP_STATUS_OK;
    int nSpeed = 230400;
    int nBits = 8;
    char nEvent = 'N';
    int nStop = 1;
	struct termios newtio,oldtio;
	if  ( tcgetattr(tp_uart_manager[inst]->handle,&oldtio)  !=  0) {
		perror("SetupSerial mcu");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch( nBits )
	{
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
	}

	switch( nEvent )
	{
		case 'O':
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			newtio.c_iflag |= (INPCK | ISTRIP);
			break;
		case 'E':
			newtio.c_iflag |= (INPCK | ISTRIP);
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			break;
		case 'N':
			newtio.c_cflag &= ~PARENB;
			break;
	}
#if 1
	switch(baudrate)
	{
		case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
		case 4800:
			cfsetispeed(&newtio, B4800);
			cfsetospeed(&newtio, B4800);
			break;
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 57600:
			cfsetispeed(&newtio, B57600);
			cfsetospeed(&newtio, B57600);
			break;
		  case 19200:
			cfsetispeed(&newtio, B19200);
			cfsetospeed(&newtio, B19200);
			break;
		case 38400:
			cfsetispeed(&newtio, B38400);
			cfsetospeed(&newtio, B38400);
			break;
		case 230400:
			cfsetispeed(&newtio, B230400);
			cfsetospeed(&newtio, B230400);
			break;
		case 460800:
			cfsetispeed(&newtio, B460800);
			cfsetospeed(&newtio, B460800);
			break;
		default:
			cfsetispeed(&newtio, B38400);
			cfsetospeed(&newtio, B38400);
			break;
	}
#endif
	if( nStop == 1 )
		newtio.c_cflag &=  ~CSTOPB;
	else if ( nStop == 2 )
	newtio.c_cflag |=  CSTOPB;
	newtio.c_cc[VTIME]  = 150;
	newtio.c_cc[VMIN] = 255;
	tcflush(tp_uart_manager[inst]->handle,TCIFLUSH);
	if((tcsetattr(tp_uart_manager[inst]->handle,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}
#endif
#if 1
    dbg_printf("-----------------------------------------baud = %d\r\n",baudrate);
	tp_status_t error = TP_STATUS_OK;
	struct termios newtio,oldtio;
	if  ( tcgetattr(tp_uart_manager[inst]->handle,&oldtio)  !=  0) {
		perror("SetupSerial mcu");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;
    int baudrate_set = 115200;
    // dbg_printf("baud_get = %d\r\n",oldtio.c_ispeed);
    // dbg_printf("B115200 = %d\r\n",B115200);
    // dbg_printf("1change baud to: %d\r\n",baudrate);
    speed_t speed;
	switch( baudrate )
	{
		case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
		case 4800:
			cfsetispeed(&newtio, B4800);
			cfsetospeed(&newtio, B4800);
			break;
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 57600:
			cfsetispeed(&newtio, B57600);
			cfsetospeed(&newtio, B57600);
			break;
		  case 19200:
			cfsetispeed(&newtio, B19200);
			cfsetospeed(&newtio, B19200);
			break;
		case 38400:
			cfsetispeed(&newtio, B38400);
			cfsetospeed(&newtio, B38400);
			break;
		case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
			break;
		case 230400:
            //printf("2change baud to: %d\r\n",baudrate);
			cfsetispeed(&newtio, B230400);
			cfsetospeed(&newtio, B230400);
			break;
		case 460800:
			cfsetispeed(&newtio, B460800);
			cfsetospeed(&newtio, B460800);
			break;
		default:
			cfsetispeed(&newtio, B230400);
			cfsetospeed(&newtio, B230400);
			break;
	}
#endif
	newtio.c_cflag |=  CSTOPB;
	newtio.c_cc[VTIME]  = 150;
	newtio.c_cc[VMIN] = 255;
	tcflush(tp_uart_manager[inst]->handle,TCIFLUSH);
	if((tcsetattr(tp_uart_manager[inst]->handle,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}
#if 1
	if  ( tcgetattr(tp_uart_manager[inst]->handle,&oldtio)  !=  0) {
		perror("SetupSerial mcu");
		return -1;
	}

    dbg_printf("baud_get = %d\r\n",oldtio.c_ispeed);
    dbg_printf("B115200 = %d\r\n",B115200);
    dbg_printf("1change baud to: %d\r\n",baudrate);
#endif
	return error;
}

tp_status_t tp_uart_flush(tp_prgdev_instance_t inst)
{
	tp_status_t error = TP_STATUS_OK;

	UART_DEBUG(("tp_uart_flush, instance = %d" TP_ENDL, inst));
	tcflush(tp_uart_manager[inst]->handle,TCIFLUSH);
	return error;
}
