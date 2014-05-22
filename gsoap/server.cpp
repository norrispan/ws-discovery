#include <signal.h>

#include "gen/wsdd.nsmap"
#include "wsddapi.h"

const int _metadataVersion = 1;
const char* _xaddr="http://localhost/service";
const char* _type="\"http://schemas.xmlsoap.org/ws/2006/02/devprof\":device";

bool stop = false;
void sighandler(int sig)
{
	std::cout<< "Signal caught..." << std::endl;
	stop = true;
}

void sendHello(soap* serv)
{
	struct soap* soap = soap_new();
	printf("call soap_wsdd_Hello\n");
	int res = soap_wsdd_Hello(soap,
	  SOAP_WSDD_ADHOC,      // mode
	  "soap.udp://239.255.255.250:3702",         // address of TS
	  soap_wsa_rand_uuid(soap),                   // message ID
	  NULL,                 
	  NULL,
	  NULL,
	  _type,
	  NULL,
	  _xaddr,
          _metadataVersion);
	if (res != SOAP_OK)
	{
		soap_print_fault(soap, stderr);
	}
	
	// listen answers
	res = soap_wsdd_listen(serv, -1000);

	if (res != SOAP_OK)
	{
		soap_print_fault(serv, stderr);
	}	
	soap_end(soap);
}

void sendBye(soap* serv)
{
	struct soap* soap = soap_new();
	printf("call soap_wsdd_Bye\n");
	int res = soap_wsdd_Bye(soap,
	  SOAP_WSDD_ADHOC,      // mode
	  "soap.udp://239.255.255.250:3702",         // address of TS
	  soap_wsa_rand_uuid(soap),                   // message ID
	  NULL,                 
	  NULL,
	  _type,
	  NULL,
	  _xaddr,
          _metadataVersion);
	if (res != SOAP_OK)
	{
		soap_print_fault(soap, stderr);
	}

	// listen answers
	res = soap_wsdd_listen(serv, -1000);

	if (res != SOAP_OK)
	{
		soap_print_fault(soap, stderr);
	}
	soap_end(soap);
}
	
int main(int argc, char** argv)
{
	const char* host = "239.255.255.250";	
	int port = 3702;
	
	// create answer listener
	struct soap* serv = soap_new1(SOAP_IO_UDP); 
	serv->bind_flags=SO_REUSEADDR;
	serv->connect_flags = SO_BROADCAST; 
	if (!soap_valid_socket(soap_bind(serv, NULL, port, 1000)))
	{
		soap_print_fault(serv, stderr);
		exit(1);
	}	
	ip_mreq mcast; 
	mcast.imr_multiaddr.s_addr = inet_addr(host);
	mcast.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(serv->master, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mcast, sizeof(mcast))<0) 
	{
		std::cout << "group membership failed:" << strerror(errno) << std::endl;		
	}		

	sendHello(serv);

	signal(SIGINT, &sighandler);
	while (!stop)
	{
		int res = soap_wsdd_listen(serv, -500000);
	}

	sendBye(serv);
	
	soap_end(serv);

	return 0;
}

void wsdd_event_ProbeMatches(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ProbeMatchesType *matches)
{
	printf("wsdd_event_ProbeMatches nbMatch:%d\n", matches->__sizeProbeMatch);
}

void wsdd_event_ResolveMatches(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ResolveMatchType *match)
{
	printf("wsdd_event_ResolveMatches\n");
}

void wsdd_event_Hello(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int MetadataVersion)
{
	printf("wsdd_event_Hello id=%s EndpointReference=%s XAddrs=%s\n", MessageID, EndpointReference, XAddrs);
}

void wsdd_event_Bye(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int *MetadataVersion)
{
	printf("wsdd_event_Bye id=%s EndpointReference=%s XAddrs=%s\n", MessageID, EndpointReference, XAddrs);
}

soap_wsdd_mode wsdd_event_Resolve(struct soap *soap, const char *MessageID, const char *ReplyTo, const char *EndpointReference, struct wsdd__ResolveMatchType *match)
{
	printf("wsdd_event_Resolve  id=%s replyTo=%s\n", MessageID, ReplyTo);
	return SOAP_WSDD_ADHOC;
}

soap_wsdd_mode wsdd_event_Probe(struct soap *soap, const char *MessageID, const char *ReplyTo, const char *Types, const char *Scopes, const char *MatchBy, struct wsdd__ProbeMatchesType *matches)
{
	printf("wsdd_event_Probe id=%s replyTo=%s types=%s scopes=%s\n", MessageID, ReplyTo, Types, Scopes);
	soap_wsdd_init_ProbeMatches(soap,matches);
	soap_wsdd_add_ProbeMatch(soap, matches, "soap.udp://239.255.255.250:3702", _type, NULL, NULL, _xaddr, _metadataVersion);
	soap_wsdd_ProbeMatches(soap, "soap.udp://239.255.255.250:3702", soap_wsa_rand_uuid(soap) , MessageID, ReplyTo, matches);
	return SOAP_WSDD_ADHOC;
}
