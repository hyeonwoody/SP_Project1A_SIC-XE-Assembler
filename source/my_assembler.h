/*
 * my_assembler �Լ��� ���� ���� ���� �� ��ũ�θ� ��� �ִ� ��� �����̴�.
 *
 */


#define MAX_INST 256
#define MAX_LINES 5000
#define MAX_OPERAND 3





#define MAX_STRLEN 1000 //buffer for every lines
#define CHECKDIR 0 //check current directory
#define INSTRUCTIONFILE 0 //check instructions (instfile.txt)
#define INPUTFILE 0//check input.txt (inputfile.txt)
#define TOKENFILE 0//check tokenized (tokenfile.txt)
#define DEBUG3 0 //DEBUG first project (HW#3 => 000000.txt) (edit today)
#define DEBUG4 1 //address in every lines

 /*
  * instruction ��� ���Ϸ� ���� ������ �޾ƿͼ� �����ϴ� ����ü �����̴�.
  * ���� ���� �ϳ��� instruction�� �����Ѵ�.
  */
struct inst_unit
{
	char str[10];
	unsigned char op;
	int format;
	int ops;
};

// instruction�� ������ ���� ����ü�� �����ϴ� ���̺� ����
typedef struct inst_unit inst;
inst* inst_table[MAX_INST];
int inst_index;

/*
 * ����� �� �ҽ��ڵ带 �Է¹޴� ���̺��̴�. ���� ������ ������ �� �ִ�.
 */
char* input_data[MAX_LINES];
static int line_num;

/*
 * ����� �� �ҽ��ڵ带 ��ū������ �����ϱ� ���� ����ü �����̴�.
 * operator�� renaming�� ����Ѵ�.
 * nixbpe�� 8bit �� ���� 6���� bit�� �̿��Ͽ� n,i,x,b,p,e�� ǥ���Ѵ�.
 */
struct token_unit
{
	char* label;
	char* operator;
	char operand[MAX_OPERAND][20];
	char comment[100];
	char nixbpe;
};

typedef struct token_unit token;
token* token_table[MAX_LINES];
static int token_line;

/*
 * �ɺ��� �����ϴ� ����ü�̴�.
 * �ɺ� ���̺��� �ɺ� �̸�, �ɺ��� ��ġ�� �����ȴ�.
 */
struct symbol_unit
{
	char* symbol;
	int addr;
	int block;
};

/*
* ���ͷ��� �����ϴ� ����ü�̴�.
* ���ͷ� ���̺��� ���ͷ��� �̸�, ���ͷ��� ��ġ�� �����ȴ�.
* ���� ������Ʈ���� ���ȴ�.
*/
struct literal_unit
{
	char* literal;
	int addr;
};

typedef struct symbol_unit symbol;
symbol sym_table[MAX_LINES];

typedef struct literal_unit literal;
literal literal_table[MAX_LINES];

static int locctr; //address counter
static int blockctr; //block counter

/*
* object code�� �����ϴ� ����ü�̴�.
* 
* 
*/
struct objectCode_unit
{
	int addr;
	int format;
	char record;
	int index;
	int code;
};
typedef struct objectCode_unit code;
code code_table[MAX_LINES];

static int index; //index for code_table


static int modi_index; //index to print only modification


//--------------

static char* input_file;
static char* output_file;
int init_my_assembler(void);
int init_inst_file(char* inst_file);
int init_input_file(char* input_file);
int token_parsing(char* str);
int search_opcode(char* str);
static int assem_pass1(void);
void make_opcode_output(char* file_name);

void make_symtab_output(char* file_name);
void make_literaltab_output(char* file_name);
static int assem_pass2(void);
void make_objectcode_output(char* file_name);

int search_symbol(char* str, int block);
int search_literal(char* str);