#define ACQUIRE_SLEEPLOCK 0
#define RELEASE_SLEEPLOCK 1
#define GET_SLEEPLOCK     2
#define REMOVE_SLEEPLOCK  3

#define ILIE -1 // invalid lock id error
#define ILSE -2 // invalid lock state error
#define NELE -3 // not enough locks error
#define WRTE -4 // wrong request type error

#define ERR_COUNT (-WRTE)
