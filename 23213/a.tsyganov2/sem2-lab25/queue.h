struct queue_record {
  struct queue_record * next, *prev;
  char buf[81];
};

typedef struct myqueue {
  struct queue_record *head, *tail;
  sem_t headsem, tailsem, queuesem;
  int beingdestroyed;
  int waiting_put;
  int waiting_get;
} queue;

void mymsginit(queue *);
void mymsgdrop(queue *);
void mymsgdestroy(queue *);
int mymsgput(queue *, char * msg);
int mymsgget(queue *, char * buf, size_t bufsize);
