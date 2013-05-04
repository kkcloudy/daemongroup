#ifndef CW_IMAGE_DATA_H
#define CW_IMAGE_DATA_H

typedef struct {
	int VerNum;
	int VerType;
	int VerLen;
	char *Ver;
	int modelLEN;
	char *model;
}CWImageIdentifier;

typedef struct {
	CWImageIdentifier *ImageRequest;
}CWImageDataRequest;
CWBool CWAssembleMsgElemImageIdentifierAC(CWProtocolMessage *msgPtr, CWImageIdentifier *resPtr);
CWBool CWAssembleResetRequestMessage(CWProtocolMessage **messagesPtr, int *fragmentsNumPtr, int PMTU, int seqNum, CWImageIdentifier *resPtr);
#endif
