#ifndef MESSAGETYPES_H
#define MESSAGETYPES_H

enum RequestContext {
    CONTEXT_STATIC = 0,    // Images, CSS, JS - Prioritize reliability
    CONTEXT_DYNAMIC = 1,   // API calls - Prioritize speed
    CONTEXT_STREAMING = 2, // Video/audio - Prioritize health
    CONTEXT_DEFAULT = 3    // Use existing strategy
};

enum MessageType {
    // DNS Messages
    DNS_QUERY = 10,
    DNS_RESPONSE = 11,

    // HTTP Messages
    HTTP_GET = 20,
    HTTP_POST = 21,
    HTTP_RESPONSE = 22,
    HTTP_ERROR = 23,

    // Control Messages
    HEALTH_CHECK = 30,
    SERVER_STATUS = 31
};

enum HttpStatus {
    HTTP_200_OK = 200,
    HTTP_404_NOT_FOUND = 404,
    HTTP_500_INTERNAL_ERROR = 500,
    HTTP_503_SERVICE_UNAVAILABLE = 503
};

#endif
