/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* asn1.h
*
*
* CREATOR:
* autelan.software.WirelessControl. team
*
* DESCRIPTION:
* asd module
*
*
*******************************************************************************/

#ifndef ASN1_H
#define ASN1_H

#define ASN1_TAG_EOC		0x00 /* not used with DER */
#define ASN1_TAG_BOOLEAN	0x01
#define ASN1_TAG_INTEGER	0x02
#define ASN1_TAG_BITSTRING	0x03
#define ASN1_TAG_OCTETSTRING	0x04
#define ASN1_TAG_NULL		0x05
#define ASN1_TAG_OID		0x06
#define ASN1_TAG_OBJECT_DESCRIPTOR	0x07 /* not yet parsed */
#define ASN1_TAG_EXTERNAL	0x08 /* not yet parsed */
#define ASN1_TAG_REAL		0x09 /* not yet parsed */
#define ASN1_TAG_ENUMERATED	0x0A /* not yet parsed */
#define ASN1_TAG_UTF8STRING	0x0C /* not yet parsed */
#define ANS1_TAG_RELATIVE_OID	0x0D
#define ASN1_TAG_SEQUENCE	0x10 /* shall be constructed */
#define ASN1_TAG_SET		0x11
#define ASN1_TAG_NUMERICSTRING	0x12 /* not yet parsed */
#define ASN1_TAG_PRINTABLESTRING	0x13
#define ASN1_TAG_TG1STRING	0x14 /* not yet parsed */
#define ASN1_TAG_VIDEOTEXSTRING	0x15 /* not yet parsed */
#define ASN1_TAG_IA5STRING	0x16
#define ASN1_TAG_UTCTIME	0x17
#define ASN1_TAG_GENERALIZEDTIME	0x18 /* not yet parsed */
#define ASN1_TAG_GRAPHICSTRING	0x19 /* not yet parsed */
#define ASN1_TAG_VISIBLESTRING	0x1A
#define ASN1_TAG_GENERALSTRING	0x1B /* not yet parsed */
#define ASN1_TAG_UNIVERSALSTRING	0x1C /* not yet parsed */
#define ASN1_TAG_BMPSTRING	0x1D /* not yet parsed */

#define ASN1_CLASS_UNIVERSAL		0
#define ASN1_CLASS_APPLICATION		1
#define ASN1_CLASS_CONTEXT_SPECIFIC	2
#define ASN1_CLASS_PRIVATE		3


struct asn1_hdr {
	const u8 *payload;
	u8 identifier, class, constructed;
	unsigned int tag, length;
};

#define ASN1_MAX_OID_LEN 20
struct asn1_oid {
	unsigned long oid[ASN1_MAX_OID_LEN];
	size_t len;
};


int asn1_get_next(const u8 *buf, size_t len, struct asn1_hdr *hdr);
int asn1_get_oid(const u8 *buf, size_t len, struct asn1_oid *oid,
		 const u8 **next);
void asn1_oid_to_str(struct asn1_oid *oid, char *buf, size_t len);
unsigned long asn1_bit_string_to_long(const u8 *buf, size_t len);

#endif /* ASN1_H */
