#define PARSE_NODE_ROOT 0
#define PARSE_NODE_SECTION 1
#define PARSE_NODE_COMPOUND_STATEMENT 2
#define PARSE_NODE_SIMPLE_STATEMENT 3
#define PARSE_NODE_DECLARATION 4
#define PARSE_NODE_ARGUMENTS 5
#define PARSE_NODE_ARGUMENT 6
#define PARSE_NODE_EXPRESSION 7
#define PARSE_NODE_L_VALUE 8
#define PARSE_NODE_R_VALUE 9
#define PARSE_NODE_OPERATOR 10


struct parse_node_section
{
//	int parsed_type;
//	int section_type;
	const char *section_name;
	int section_name_len;
};

struct parse_node_identifier
{
//	int storage_type;
	const char *id_name;
	int id_name_len;
};

union parse_data
{
//	int parsed_type;
	struct parse_node_section section;
	struct parse_node_identifier identifier;
//	struct parse_node_section section;
};

// A node of the parse tree.
struct parse_node
{
	dword token_num;
	qword node_type;
	qword parent;
	union parse_data data;
};
