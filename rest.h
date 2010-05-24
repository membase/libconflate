#ifndef REST_H
#define	REST_H

#define RESPONSE_BUFFER_SIZE 4096
#define DEFAULT_BUCKET "/pools/default/buckets/default"
#define DEFAULT_BUCKET_STREAM "/pools/default/bucketsStreaming/default"
#define END_OF_CONFIG "\n\n\n\n"
#define CONFIG_KEY "contents"

void* run_rest_conflate(void *arg);

#endif	/* REST_H */

