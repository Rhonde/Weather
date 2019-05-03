
/*----------------------------------------------------------------------------
  Notes:
  The length of the receive and transmit buffers must be a power of 2.
  Each buffer has a next_in and a next_out index.
  If next_in = next_out, the buffer is empty.
  (next_in - next_out) % buffer_size = the number of characters in the buffer.
 *----------------------------------------------------------------------------*/
#define TBUF_SIZE   256	     /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/
#define RBUF_SIZE   256      /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/

/*----------------------------------------------------------------------------
 *----------------------------------------------------------------------------*/
#if TBUF_SIZE < 2
#error TBUF_SIZE is too small.  It must be larger than 1.
#elif ((TBUF_SIZE & (TBUF_SIZE-1)) != 0)
#error TBUF_SIZE must be a power of 2.
#endif

#if RBUF_SIZE < 2
#error RBUF_SIZE is too small.  It must be larger than 1.
#elif ((RBUF_SIZE & (RBUF_SIZE-1)) != 0)
#error RBUF_SIZE must be a power of 2.
#endif

/*----------------------------------------------------------------------------
 *----------------------------------------------------------------------------*/
typedef struct _buf_st {
  unsigned int in;                                // Next In Index
  unsigned int out;                               // Next Out Index
  char buf [RBUF_SIZE];                           // Buffer
} buf_st;

static buf_st rxBuf = { 0, 0, };
// #define SIO_RBUFLEN ((unsigned short)(rbuf.in - rbuf.out))

static buf_st txBuf = { 0, 0, };
// #define SIO_TBUFLEN ((unsigned short)(tbuf.in - tbuf.out))

static unsigned int tx_restart = 1;               // NZ if TX restart is required

/*----------------------------------------------------------------------------
  USART1_IRQHandler
  Handles USART1 global interrupt request.
 *----------------------------------------------------------------------------*/
void USART2_IRQHandler (void) {
  volatile unsigned int IIR;
  struct buf_st *p;

    IIR = USART2->SR;		// read uasrt status register
    if (IIR & USART_FLAG_RXNE) {                  // read interrupt
      USART2->SR &= ~USART_FLAG_RXNE;	          // clear interrupt

      p = &rbuf;

      if (((p->in - p->out) & ~(RBUF_SIZE-1)) == 0) {
        p->buf [p->in & (RBUF_SIZE-1)] = (USART1->DR & 0x1FF);
        p->in++;
      }
    }

    if (IIR & USART_FLAG_TXE) {
      USART1->SR &= ~USART_FLAG_TXE;	          // clear interrupt

      p = &tbuf;

      if (p->in != p->out) {
        USART1->DR = (p->buf [p->out & (TBUF_SIZE-1)] & 0x1FF);
        p->out++;
        tx_restart = 0;
      }
      else {
        tx_restart = 1;
		USART1->CR1 &= ~USART_FLAG_TXE;		      // disable TX interrupt if nothing to send

      }
    }
}
/*------------------------------------------------------------------------------
 *	bool USART2_IsEmpty()
 * 	returns true is buffer is empty
------------------------------------------------------------------------------*/
bool USART2_IsEmpty(buf_st *pBuf)
{
	if((pBuf->in - pBuf->out) == 0)
		return true;
}
/*------------------------------------------------------------------------------
  buffer_Init
  initialize the buffers
 *------------------------------------------------------------------------------*/
void buffer_Init (void) {

  tbuf.in = 0;                                    // Clear com buffer indexes
  tbuf.out = 0;
  tx_restart = 1;

  rbuf.in = 0;
  rbuf.out = 0;
}

/*------------------------------------------------------------------------------
  SendByte
  transmit a character
 *------------------------------------------------------------------------------*/
int SendByte (byte c) {
  struct buf_st *p = &tbuf;

                                                  // If the buffer is full, return an error value
  if (SIO_TBUFLEN >= TBUF_SIZE)
    return (-1);

  p->buf [p->in & (TBUF_SIZE - 1)] = c;           // Add data to the transmit buffer.
  p->in++;

  if (tx_restart) {                               // If transmit interrupt is disabled, enable it
    tx_restart = 0;
	USART1->CR1 |= USART_FLAG_TXE;		          // enable TX interrupt
  }

  return (0);
}

/*------------------------------------------------------------------------------
  GetKey
  receive a character
 *------------------------------------------------------------------------------*/
int GetKey (void) {
  struct buf_st *p = &rbuf;

  if (SIO_RBUFLEN == 0)
    return (-1);

  return (p->buf [(p->out++) & (RBUF_SIZE - 1)]);
}
