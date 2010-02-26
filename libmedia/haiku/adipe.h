#ifndef __ADIPE_H_
#define __ADIPE_H_


#define QQ_LVL 6

#define QQ(x)	do { 							\
			if ( (x) <= QQ_LVL )				\
				fprintf(stderr,"QQ %s \t%d \t%s\n",	\
					__FILE__, __LINE__,		\
					 __PRETTY_FUNCTION__);      	\
		} while(0)

#endif

