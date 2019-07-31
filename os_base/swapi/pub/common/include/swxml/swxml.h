#ifndef __SW_XML_H__
#define __SW_XML_H__

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

/* Node Type */
typedef enum
{
	eINVALID_NODE                   = 0,
	eELEMENT_NODE                   = 1,
	eATTRIBUTE_NODE                 = 2,
	eTEXT_NODE                      = 3,
	eCDATA_SECTION_NODE             = 4,
	eENTITY_REFERENCE_NODE          = 5,
	eENTITY_NODE                    = 6,                
	ePROCESSING_INSTRUCTION_NODE    = 7,
	eCOMMENT_NODE                   = 8,
	eDOCUMENT_NODE                  = 9,
	eDOCUMENT_TYPE_NODE             = 10,
	eDOCUMENT_FRAGMENT_NODE         = 11,
	eNOTATION_NODE                  = 12
} XML_NODE_TYPE;

/* Error */
typedef enum 
{
	XML_INDEX_SIZE_ERR                 = 1,
	XML_DOMSTRING_SIZE_ERR             = 2,
	XML_HIERARCHY_REQUEST_ERR          = 3,
	XML_WRONG_DOCUMENT_ERR             = 4,
	XML_INVALID_CHARACTER_ERR          = 5,
	XML_NO_DATA_ALLOWED_ERR            = 6,
	XML_NO_MODIFICATION_ALLOWED_ERR    = 7,
	XML_NOT_FOUND_ERR                  = 8,
	XML_NOT_SUPPORTED_ERR              = 9,
	XML_INUSE_ATTRIBUTE_ERR            = 10,
	XML_INVALID_STATE_ERR              = 11,
	XML_SYNTAX_ERR                     = 12,
	XML_INVALID_MODIFICATION_ERR       = 13,
	XML_NAMESPACE_ERR                  = 14,
	XML_INVALID_ACCESS_ERR             = 15,

	XML_SUCCESS                        = 0,
	XML_NO_SUCH_FILE                   = 101,
	XML_INSUFFICIENT_MEMORY            = 102,
	XML_FILE_DONE                      = 104,
	XML_INVALID_PARAMETER              = 105,
	XML_FAILED                         = 106,
	XML_INVALID_ITEM_NUMBER            = 107
} XML_ERRORCODE;

#define DOCUMENTNODENAME    "#document"
#define TEXTNODENAME        "#text"
#define CDATANODENAME       "#cdata-section"

typedef struct _XML_Document	*Docptr;
typedef struct _XML_Node			*Nodeptr;

typedef struct _XML_Node
{
	char* nodeName;
	char* nodeValue;
	XML_NODE_TYPE nodeType;
	char* namespaceURI;
	char* prefix;
	char* localName;
	BOOL readOnly;
	Nodeptr parentNode;
	Nodeptr firstChild;
	Nodeptr prevSibling;
	Nodeptr nextSibling;
	Nodeptr firstAttr;

	//Element
	char* tagName;

	//Attribute
	BOOL specified;
	Nodeptr owner;
} XML_Node;

typedef XML_Node XML_Document;
typedef XML_Node XML_CDATASection;
typedef XML_Node XML_Element;
typedef XML_Node XML_Attr;

typedef struct _XML_NodeList
{
	Nodeptr nodeItem;
	struct _XML_NodeList *next;
} XML_NodeList;

typedef struct _XML_NamedNodeMap
{
	Nodeptr nodeItem;
	struct _XML_NamedNodeMap *next;
} XML_NamedNodeMap;

//extern HANDLE g_xml_mem;

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @brief parser xml �ϸ���xml�﷨�ڵ㶼��Ҫ�պ�
 * 		��Ҫ����sw_xml_freeNode�ͷ������еĽڵ��ڴ�
 * 
 * @param buffer 
 * 
 * @return 
 */
XML_Node* sw_xml_toNode( const char* buffer );
/**
 * @brief ����ڵ�ֻ��һ����Բ��ñպ�
 */
XML_Node* sw_xml_toNode_UnClosed( const char* buffer);

/**
 * @brief ��xml�ڵ�תΪ�ַ�������Ҫ���÷��ͷ��ڴ�
 */
char* sw_xml_toString( XML_Node* node );

/** 
 * @brief create new xml node--������װxml�ļ�
 * 
 * @param ns 
 * @param name 
 * @param value 
 * @param result 
 * 
 * @return 
 */
int sw_xml_createNode( const char* ns, const char* name, const char* value, XML_Node** result );

/** 
 * @brief frees all nodes under nodeptr subtree
 * 
 * @param node 
 */
void sw_xml_freeNode( XML_Node* node );

/** 
 * @brief --������װxml�ļ�
 * 
 * @param parent 
 * @param node 
 * 
 * @return 
 */
int sw_xml_appendChildNode( XML_Node* parent, XML_Node* node );

/** 
 * @brief --������װxml�ļ�
 * 
 * @param node 
 * @param ns 
 * @param name 
 * @param value 
 * 
 * @return 
 */
int sw_xml_addAttribute( XML_Node* node, const char* ns, const char* name, const char* value );

/** 
 * @brief---------must do not call free
 * 
 * @param node 
 * @param type <-------- node->nodeType
 * @param ns <-----------node->prefix
 * @param name <---------node->localName
 * @param value <--------node->firstChild->nodeValue;
 * 
 * @return 
 */
int sw_xml_getNodeValue( XML_Node* node, XML_NODE_TYPE* type, const char** ns, /*const*/ char** name, /*const*/ char** value );

/** 
 * @brief �Ӹ��ڵ�ƴ�ӵ�ַroot.FRST.SECD.THRD.FOUTH.FIVE.**ƴ�ӵ���ǰ�ڵ�Ȼ�������Ա�url��ַ,����ڵ������ظ������ƴȫurl��ַ
 * 
 * @param root ��ô����Ǹ��ڵ��ַ
 * @param url<------SECD.THRD
 * @param node 
 * 
 * @return 
 */
int sw_xml_getNodeByUrl( XML_Node* root, const char* url, XML_Node** node );

/** 
 * @brief ͨ��sw_xml_getNodeByUrl���ҵ���Ӧ�Ľڵ�ȡ����ڵ�ĵ�һ�ӽڵ�nodeValue
 * 
 * @param root 
 * @param url 
 * @param value -----must do not call free
 * 
 * @return 
 */
int sw_xml_getValueByUrl( XML_Node* root, const char* url, char** value );

/** 
 * @brief ��ȡ��ǰ�ڵ���ӽڵ���
 * 
 * @param node
 * @param count 
 * 
 * @return 
 */
int sw_xml_getNodeChildNum( XML_Node* node, int* count );

/** 
 * @brief ��ȡ��һ�ӽڵ�
 * 
 * @param node 
 * 
 * @return 
 */
XML_Node* sw_xml_getNodeFirstChild( XML_Node* node );

/** 
 * @brief ��ȡ��ǰ�ڵ���ֵܽڵ�
 * 
 * @param node 
 * 
 * @return 
 */
XML_Node* sw_xml_getNodeNextSibling( XML_Node* node );

/** 
 * @brief ��ȡ��ǰ�ڵ�ĵ�һ���Խڵ�,���������Կ������ֵܽڵ��ȡ����
 * 
 * @param node 
 * 
 * @return 
 */
XML_Node* sw_xml_getNodeFirstAttr( XML_Node* node );

/** 
 * @brief ���ҵ�ǰ�ڵ��url��Ӧ������ֵ��
 * 
 * @param node 
 * @param url
 * @param ns <-----------n->prefix
 * @param value <--------n->nodeValue, must do not call free
 * 
 * @return 
 */
int sw_xml_getNodeAttrByUrl( XML_Node* node, const char *url, char **ns, char **value);

#ifdef __cplusplus
}
#endif

#endif
