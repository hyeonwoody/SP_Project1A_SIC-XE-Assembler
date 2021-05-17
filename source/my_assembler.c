/*
 * 화일명 : my_assembler_000000.c
 * 설  명 : 이 프로그램은 SIC/XE 머신을 위한 간단한 Assembler 프로그램의 메인루틴으로,
 * 입력된 파일의 코드 중, 명령어에 해당하는 OPCODE를 찾아 출력한다.
 * 파일 내에서 사용되는 문자열 "000000"에는 자신의 학번을 기입한다.
 */

 /*
  *
  * 프로그램의 헤더를 정의한다.
  *
  */

#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>


  // 파일명의 "000000"은 자신의 학번으로 변경할 것.
#include "my_assembler.h"

/* ----------------------------------------------------------------------------------
 * 설명 : 사용자로 부터 어셈블리 파일을 받아서 명령어의 OPCODE를 찾아 출력한다.
 * 매계 : 실행 파일, 어셈블리 파일
 * 반환 : 성공 = 0, 실패 = < 0
 * 주의 : 현재 어셈블리 프로그램의 리스트 파일을 생성하는 루틴은 만들지 않았다.
 *		   또한 중간파일을 생성하지 않는다.
 * ----------------------------------------------------------------------------------
 */
int main(int args, char* arg[])
{

	if (init_my_assembler() < 0)
	{
		printf("init_my_assembler: 프로그램 초기화에 실패 했습니다.\n");
		return -1;
	}

	if (assem_pass1() < 0)
	{
		printf("assem_pass1: 패스1 과정에서 실패하였습니다.  \n");
		return -1;
	}

	make_opcode_output("output_000000");

	/*
				 NEW
	 */
	make_symtab_output("symtab_000000");
	make_literaltab_output("literaltab_000000");
	if (assem_pass2() < 0)
	{
		printf(" assem_pass2: 패스2 과정에서 실패하였습니다.  \n");
		return -1;
	}

	make_objectcode_output("output_000000");

	return 0;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 프로그램 초기화를 위한 자료구조 생성 및 파일을 읽는 함수이다.
 * 매계 : 없음
 * 반환 : 정상종료 = 0 , 에러 발생 = -1
 * 주의 : 각각의 명령어 테이블을 내부에 선언하지 않고 관리를 용이하게 하기
 *		   위해서 파일 단위로 관리하여 프로그램 초기화를 통해 정보를 읽어 올 수 있도록
 *		   구현하였다.
 * ----------------------------------------------------------------------------------
 */
int init_my_assembler(void)
{
	int result;

	if ((result = init_inst_file("inst.data")) < 0)
		return -1;
	if ((result = init_input_file("input.txt")) < 0)
		return -1;
	return result;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 머신을 위한 기계 코드목록 파일을 읽어 기계어 목록 테이블(inst_table)을
 *        생성하는 함수이다.
 * 매계 : 기계어 목록 파일
 * 반환 : 정상종료 = 0 , 에러 < 0
 * 주의 : 기계어 목록파일 형식은 자유롭게 구현한다. 예시는 다음과 같다.
 *
 *	===============================================================================
 *		   | 이름 | 형식 | 기계어 코드 | 오퍼랜드의 갯수 | NULL|
 *	===============================================================================
 *
 * ----------------------------------------------------------------------------------
 */
int init_inst_file(char* inst_file)
{
	FILE* file;
	int errno;
	/* add your code here */
	errno = 0; // no error

#if CHECKDIR //현재 경로를 파악하기 위한 매크로
	char cwd[1000];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
		printf("dir : %s\n", cwd);
#endif

	if ((file = fopen(inst_file, "r")) == NULL) {
		errno = -1;//error : cant open the file
		return errno;
	}

	inst_index = 0; //index for commands from .h

	char str[10] = ""; //temporary 
	unsigned char op = 0; //tmp
	int format = 0; //tmp
	int ops = 0; //tmp

	while ((fscanf(file, "%s\t%hhX\t%d\t%d", str, &op, &format, &ops) != EOF)) {
		inst_table[inst_index] = (inst*)malloc(sizeof(inst)); //initialize inst_table
		memset(inst_table[inst_index], 0, sizeof(inst));

		strcpy(inst_table[inst_index]->str, str); //save the data
		inst_table[inst_index]->op = op;
		inst_table[inst_index]->format = format;
		inst_table[inst_index++]->ops = ops;

		if (inst_index == MAX_INST) { //reached maximum number of instruction
			if ((fgetc(file) != EOF)) {
				errno = -2; // error : instruction exceed maximum
			}
			break;
		}
	}

	fclose(file);

#if INSTRUCTIONFILE
	if ((file = fopen("instfile.txt", "w")) == NULL) {
		errno = -1;//error : cant open the file
		return errno;
	}
	int i = 0;
	do {
		fprintf(file, "%s\t", inst_table[i]->str);
		fprintf(file, "%02X\t", inst_table[i]->op); //02X=>if less than 10, puts 0 in front
		fprintf(file, "%d\t", inst_table[i]->format);
		fprintf(file, "%d\n", inst_table[i++]->ops);
	} while (i < inst_index);
	fclose(file);
#endif
	return errno;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 어셈블리 할 소스코드를 읽어 소스코드 테이블(input_data)를 생성하는 함수이다.
 * 매계 : 어셈블리할 소스파일명
 * 반환 : 정상종료 = 0 , 에러 < 0
 * 주의 : 라인단위로 저장한다.
 *
 * ----------------------------------------------------------------------------------
 */
int init_input_file(char* input_file)
{
	FILE* file;
	int errno;

	/* add your code here */
	if ((file = fopen(input_file, "r")) == NULL) {
		errno = -1;//error : cant open the file
		return errno;
	}

	line_num = 0; //line counter from .h

	char tmp[MAX_STRLEN] = ""; //temporary string

	while ((fscanf(file, "%[^\n]", tmp) != EOF)) {
		input_data[line_num] = (char*)malloc((strlen(tmp) + 1) * sizeof(char)); //extra +1 for NULL
		memset(input_data[line_num], 0, (strlen(tmp) + 1) * sizeof(char));

		strcpy(input_data[line_num++], tmp);
		//fseek(file, 1, SEEK_CUR);
		fgetc(file);
		if (line_num == MAX_LINES) { //reached maximum number of instruction
			if ((fgetc(file) != EOF)) {
				errno = -2; // error : instruction exceed maximum
			}
			break;
		}
	}

	fclose(file);

#if INPUTFILE
	if ((file = fopen("inputfile.txt", "w")) == NULL) {
		errno = -3;//error : cant open the file
		return errno;
	}

	for (int i = 0; i < line_num; i++) {
		fprintf(file, "%s\n", input_data[i]);
	}
	fclose(file);
#endif

	errno = 0;
	return errno;
}

/* ----------------------------------------------------------------------------------
 * 설명 : 소스 코드를 읽어와 토큰단위로 분석하고 토큰 테이블을 작성하는 함수이다.
 *        패스 1로 부터 호출된다.
 * 매계 : 파싱을 원하는 문자열
 * 반환 : 정상종료 = 0 , 에러 < 0
 * 주의 : my_assembler 프로그램에서는 라인단위로 토큰 및 오브젝트 관리를 하고 있다.
 * ----------------------------------------------------------------------------------
 */
int token_parsing(char* str)
{
	/* add your code here */
	int errno;
	errno = 0;
	static char ch = ' ';
	char* cmp; //cmp = compare with command

	int num_operand = 0;
	// for (int i=0; i < 3; i++){
	// 	memset(token_table[token_line]->operand[i], 0, (20)*sizeof(char)); //initialize
	// }

	int oI = 0; //Operand Index

	token_table[token_line] = (token*)malloc(sizeof(token));
	memset(token_table[token_line], 0, sizeof(token));

	for (int type = 0; type < 4; type++) {
		int j = 0;

		cmp = (char*)malloc((strlen(str) + 1) * sizeof(char));
		memset(cmp, 0, (strlen(str) + 1) * sizeof(char));

		while (ch != '\t' && ch != ',' && ch != '\r' && *(str) != '\0') {
			ch = *(str++);
			cmp[j++] = ch;
		}

		switch (type) { //type => 0:label , 1:operator, 2:operand, 3:comment

		case 0: //label
			if (*(str) != '\0')
				cmp[j - 1] = '\0';
			if (cmp[0] == '.') {
				token_table[token_line]->operator = (char*)malloc(1 * sizeof(char));
				memset(token_table[token_line]->operator, 0, 1 * sizeof(char)); //add
				type = 2; //next token is for comment
			}
			token_table[token_line]->label = (char*)malloc((strlen(cmp) + 1) * sizeof(char));
			memset(token_table[token_line]->label, 0, (strlen(cmp) + 1) * sizeof(char));

			if (cmp[0] != '\t') {
				strcpy(token_table[token_line]->label, cmp);
			}
			//fprintf (stdout,"%s\t", token_table[token_line]->label);
			break;
		case 1: //operator
			if (*(str) != '\0')
				cmp[j - 1] = '\0';

			token_table[token_line]->operator = (char*)malloc((strlen(cmp) + 1) * sizeof(char));
			memset(token_table[token_line]->operator, 0, (strlen(cmp) + 1) * sizeof(char));
			strcpy(token_table[token_line]->operator, cmp);

			// token_table[token_line]->operator = (char*)malloc((strlen(cmp)+1)*sizeof(char));
			// 	if ( !( 65 <= *(cmp) && *(cmp) <= 90)){ //if operator doesnt start with letter
			// 		cmp++;
			// 	} //4 byte expended
			// for (int i=0; i < inst_index; i++){
			// 	if (!(strcmp (cmp, inst_table[i]->str))){
			// 		//strcpy(token_table[token_line]->operator, inst_table[i]->str);
			// 		strcpy(token_table[token_line]->operator, --cmp);
			// 	}
			// }
			break;
		case 2: //operand
			if (cmp[j - 1] == ',') {
				type--; //if there is multiple operands, do it again
			}
			if (*(str) != '\0')
				cmp[j - 1] = '\0';

			memset(token_table[token_line]->operand[num_operand], 0, (20) * sizeof(char));
			strcpy(token_table[token_line]->operand[num_operand++], cmp);

			//fprintf (stdout,"%s\t", token_table[token_line]->operand[num_operand-1]);
			break;
		case 3: //comment
			if (*(str) != '\0')
				cmp[j - 1] = '\0';

			memset(token_table[token_line]->comment, 0, (100) * sizeof(char));
			strcpy(token_table[token_line]->comment, cmp);

			//fprintf (stdout,"%s\n", token_table[token_line]->comment);
			break;
		}

		ch = ' ';


	}

	free(cmp);


	return errno;
	// do{
	// while (ch=='\t'||ch=='\r'){
	//     ch = *(str++);
	// }
	// 	tmp = (char *) malloc ((strlen(str)+1)*sizeof(char));
	// 	tmp
	// }
	// while (str!='\0');
}

/* ----------------------------------------------------------------------------------
 * 설명 : 입력 문자열이 기계어 코드인지를 검사하는 함수이다.
 * 매계 : 토큰 단위로 구분된 문자열
 * 반환 : 정상종료 = 기계어 테이블 인덱스, 에러 < 0
 * 주의 :
 *
 * ----------------------------------------------------------------------------------
 */
int search_opcode(char* str)
{
	/* add your code here */
	if (str == NULL)
		return -2;

	if (str[0] == '+')
		str++;
	for (int i = 0; i < inst_index; i++) {
		if (strcmp(str, inst_table[i]->str) == 0)
			return i; //specific index
	}

	return -1;
}

int search_symbol(char* str, int block)
{
	int had = 0;
	if (strcmp(str, "\0") == 0) {
		return -2;
	}
	if (str[0] == '.') {
		return -3;
	}
	for (int i = 0; i < line_num; i++) {
		if (sym_table[i].symbol != NULL && strcmp(str, sym_table[i].symbol) == 0) {
			if (block == sym_table[i].block)
				return i; //if exist, return larger than -1
			else {
				had = 1;
				continue;
			}
			//if exist in different block, return -1
		}
	}

	if (had) {
		return -1;
	}

	return -4; //if not exist, return -4
}

int search_literal(char* str) {
	if (strcmp(str, "\0") == 0)
		return -2;
	for (int i = 0; i < line_num; i++) {
		if (literal_table[i].literal != NULL && strcmp(str, literal_table[i].literal) == 0)
			return i;
	}
	return -1; //if not exist, return -1
}

/* ----------------------------------------------------------------------------------
* 설명 : 어셈블리 코드를 위한 패스1과정을 수행하는 함수이다.
*		   패스1에서는..
*		   1. 프로그램 소스를 스캔하여 해당하는 토큰단위로 분리하여 프로그램 라인별 토큰
*		   테이블을 생성한다.
*
* 매계 : 없음
* 반환 : 정상 종료 = 0 , 에러 = < 0
* 주의 : 현재 초기 버전에서는 에러에 대한 검사를 하지 않고 넘어간 상태이다.
*	  따라서 에러에 대한 검사 루틴을 추가해야 한다.
*
* -----------------------------------------------------------------------------------
*/
static int assem_pass1(void)
{
	/* add your code here */
	/* input_data의 문자열을 한줄씩 입력 받아서
	 * token_parsing()을 호출하여 token_unit에 저장
	 */
	int result; //return from token_parsing and search_opcode
	int errno;
	errno = 0;

	token_line = 0; //from .h

	blockctr = 0; //from .h


	char START[10];
	memset(START, 0, sizeof(char) * 10);

	for (int i = 0; i < line_num; i++) {

		//if not allocated, label and operator share the same address
		if ((result = token_parsing(input_data[i])) < 0) {
			errno = -1;
		}

#if DEBUG4
		sym_table[i].addr = locctr;
#endif // DEBUG4

		//dividing blocks
		if (strcmp(token_table[i]->operator, "CSECT") == 0) {
			blockctr++;
			locctr = 0;
		}

		//writing symbol table
		switch (search_symbol(token_table[i]->label, blockctr)) {
		case -2: //label is blank
			break;
		case -3: //label is '.'
			break;
		case -4: //label is new
		case -1: //label exist in different block
			sym_table[i].symbol = (char*)malloc((strlen(token_table[i]->label) + 1) * sizeof(char));
			memset(sym_table[i].symbol, 0, (strlen(token_table[i]->label) + 1) * sizeof(char));
			strcpy(sym_table[i].symbol, token_table[i]->label);
			sym_table[i].addr = locctr;
			break;
		default: //label is already exist in symbol table
			fprintf(stdout, "assem_pass1: duplicate symbol : %s \n", token_table[i]->label);
			break;
		}
		//end of writing symbol

	//addint to literalpool
		if (token_table[i]->operand[0][0] == '=') {
			if ((result = search_literal(token_table[i]->operand[0])) == -1) { // check literal table
				//if not exist
				literal_table[i].literal = (char*)malloc((strlen(token_table[i]->operand[0]) + 1) * sizeof(char));
				memset(literal_table[i].literal, 0, (strlen(token_table[i]->operand[0]) + 1) * sizeof(char));
				strcpy(literal_table[i].literal, token_table[i]->operand[0]); //add to literal table but not assigning address
				literal_table[i].addr = -1;
			}
		}
		//end of adding to literalpool

		//calculating next address
		if ((result = search_opcode(token_table[i]->operator)) > -1) {//according to format
			locctr += inst_table[result]->format;
			if (token_table[i]->operator[0] == '+') {
				locctr++;
			}
		}
		else if (strcmp(token_table[i]->operator, "RESW") == 0) {
			locctr += 3 * atoi(token_table[i]->operand[0]);
		}
		else if (strcmp(token_table[i]->operator, "RESB") == 0) {
			locctr += atoi(token_table[i]->operand[0]);
		}
		else if (token_table[i]->operator[0] != '\0' && strcmp(token_table[i]->operator, "BYTE") == 0) {
			if (token_table[i]->operand[0][0] == 'X') {
				locctr += (strlen(token_table[i]->operand[0]) - 3) / 2; //except X'', 2byte 
			}
		}
		else if (strcmp(token_table[i]->operator, "EQU") == 0) {
			if (strcmp(token_table[i]->operand[0], "*") == 0) {
				//symbol is already in the symbol table & locctr incremnet not required
			}
			else {
				char* first;
				char* second;

				int first_symbol;
				int second_symbol;
				int index = 0; //index for first symbol
				int j = 0;
				first = (char*)malloc((strlen(token_table[i]->operand[0]) + 1) * sizeof(char));
				memset(first, 0, (strlen(token_table[i]->operand[0]) + 1) * sizeof(char));
				strcpy(first, token_table[i]->operand[0]);

				for (int i = 0; first[i] != '-' || first[i] == NULL; i++) {
					index++;
				}
				if (index < strlen(token_table[i]->operand[0])) {
					index++;
					second = (char*)malloc((strlen(first + index) + 1) * sizeof(char));
					memset(second, 1, (strlen(first + index) + 1) * sizeof(char));
					for (int i = index; second[j] != NULL; i++) {
						second[j++] = first[i];
					}
					first[index - 1] = '\0';
					first_symbol = search_symbol(first, blockctr);
					second_symbol = search_symbol(second, blockctr);

					sym_table[i].addr = sym_table[first_symbol].addr - sym_table[second_symbol].addr;
					free(second);
				}
				else {
					locctr += 3;
				}
				free(first);

			}
		}
		else if (strcmp(token_table[i]->operator, "WORD") == 0) {
			locctr += 3;
		}
		else if ((strcmp(token_table[i]->operator, "LTORG") == 0) || //literal save
			(strcmp(token_table[i]->operator, "END") == 0)) {

			for (int j = i; j >= 0; j--) {
				if (literal_table[j].addr == -1) {
					literal_table[j].addr = locctr;
					if (literal_table[j].literal[1] == 'X')
						locctr += (strlen(literal_table[j].literal) - 4) / 2;
					else
						locctr += (strlen(literal_table[j].literal) - 4);
				}
			}

		}
		else {
			errno = -3;
			fprintf(stdout, "assem_pass1: invalid operation code : %s \n", token_table[i]->operator);
		}
		//end of calculating next address

		sym_table[i].block = blockctr;
		token_line++;
	}


#if TOKENFILE
	FILE* file;

	int oI = 0;

	file = fopen("tokenfile.txt", "w");

	for (int i = 0; i < line_num; i++) {
		fprintf(file, "%s\t", token_table[i]->label);
		fprintf(file, "%s\t", token_table[i]->operator);
		while (oI < 3 && token_table[i]->operand[oI][0] != '\0') {
			fprintf(file, "%s,", token_table[i]->operand[oI++]);
		}
		oI = 0;
		fseek(file, -1, SEEK_CUR);
		fprintf(file, "\t%s\n", token_table[i]->comment);
	}
	fclose(file);
#endif

	return errno;
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 명령어 옆에 OPCODE가 기록된 표(과제 3번) 이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*        또한 과제 3번에서만 쓰이는 함수이므로 이후의 프로젝트에서는 사용되지 않는다.
* -----------------------------------------------------------------------------------
*/
void make_opcode_output(char* file_name)
{
	/* add your code here */
	FILE* file;

	int result;

	int oI = 0; //operand_index
	if (file_name == NULL) {
		file = stdout;//stdout
	}

	else {
		file = fopen(file_name, "w");
	}
#if DEBUG3
	fclose(file);
	file = fopen("000000.txt", "w");
	fprintf(file, "today : 20210426\n");
#endif
	//token : label operator operand opcode
	for (int i = 0; i < token_line; i++) {
		fprintf(file, "%04X\t", sym_table[i].addr);
		fprintf(file, "%s\t", token_table[i]->label); //label

		fprintf(file, "%s", token_table[i]->operator);
		if (token_table[i]->label[0] == '.') { //comment .
			fprintf(file, "%s\n", token_table[i]->comment);
			continue;
		}
		fprintf(file, "\t");
		while (oI < 3) {
			fprintf(file, "%s", token_table[i]->operand[oI++]);
			if (token_table[i]->operand[oI][0] == '\0') {
				oI = 0;
				break;
			}
			fprintf(file, ",");
		}

		fprintf(file, "\t\t");
		// if (i != 52 && i != 36){
		// 	fprintf (file, "\t"); //prevent not allign
		// }
		if ((result = search_opcode(token_table[i]->operator)) != -1) {
			fprintf(file, "%02X", inst_table[result]->op);
		}

		fprintf(file, "\n");
	}
	fclose(file);
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 SYMBOL별 주소값이 저장된 TABLE이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*
* -----------------------------------------------------------------------------------
*/
void make_symtab_output(char* file_name)
{
	/* add your code here */

	FILE* file;
	int block = 0;
	if (file_name == NULL) {
		file = stdout;//stdout
	}

	else {
		file = fopen(file_name, "w");
	}
	for (int i = 0; i < line_num; i++) {
		if (sym_table[i].symbol != NULL) {
			if (block < sym_table[i].block)
				fprintf(file, "\n");
			fprintf(file, "%s\t\t %X\n", sym_table[i].symbol, sym_table[i].addr);
			block = sym_table[i].block;
		}
	}

	fclose(file);
}

void make_literaltab_output(char* file_name)
{
	/* add your code here */
	FILE* file;
	int block = 0;
	char* tmp;
	char* ptr;
	if (file_name == NULL) {
		file = stdout;//stdout
	}
	else {
		file = fopen(file_name, "w");
	}

	for (int i = 0; i < line_num; i++) {
		if (literal_table[i].literal != NULL) {

			ptr = literal_table[i].literal + 3;
			tmp = (char*)malloc((strlen(literal_table[i].literal) - 4 + 1) * sizeof(char)); //-4 => =C''
			memset(tmp, 0, ((strlen(literal_table[i].literal) - 4 + 1) * sizeof(char)));
			for (int j = 0; j < strlen(literal_table[i].literal) - 4; j++) {
				tmp[j] = *(ptr++);
			}
			fprintf(file, "%s\t\t %X\n", tmp, literal_table[i].addr);
			free(tmp);
		}
	}

	fclose(file);
}

/* ----------------------------------------------------------------------------------
* 설명 : 어셈블리 코드를 기계어 코드로 바꾸기 위한 패스2 과정을 수행하는 함수이다.
*		   패스 2에서는 프로그램을 기계어로 바꾸는 작업은 라인 단위로 수행된다.
*		   다음과 같은 작업이 수행되어 진다.
*		   1. 실제로 해당 어셈블리 명령어를 기계어로 바꾸는 작업을 수행한다.
* 매계 : 없음
* 반환 : 정상종료 = 0, 에러발생 = < 0
* 주의 :
* -----------------------------------------------------------------------------------
*/
static int assem_pass2(void) {
	locctr = 0;
	blockctr = 0;

	int H_index = 0;

	int E_addr = 0;

	char EXTREF[3][MAX_STRLEN] = { 0, };

	char* tmp;

	int result = 0;

	int length = 0; //last code's format (=length)





	int i = 0;
	for (i = 0; i < token_line; i++) {

		
		/// <summary>
		/// Beginning of program 
		/// </summary>
		/// <param name=""></param>
		/// <returns></returns>
		 
		

		if (strcmp(token_table[i]->operator, "START") == 0 ||
			strcmp(token_table[i]->operator, "CSECT") == 0) {


			E_addr = sym_table[i].addr;
			int scan = i + 1;

			for (scan; scan < line_num; scan++) {
				if (strcmp(token_table[scan]->operator, "CSECT") == 0)
					break;
				if (E_addr < sym_table[scan].addr)
					E_addr = sym_table[scan].addr;
			}
			if (strcmp(token_table[i]->operator, "START") != 0) {
				code_table[index].format = 0;
				code_table[index].record = 'E';
				code_table[index].index = i;
				index++;
				blockctr++;
			}
			code_table[index].addr = E_addr; //figureout the length of end_addr later
			//saving record

			code_table[index].format = 0;
			code_table[index].record = 'H';
			code_table[index].index = i;

			H_index = index;

		}

		if (locctr) {
			code_table[index].addr += locctr;
			locctr = 0;
		}

		else if (strcmp(token_table[i]->operator, "EXTDEF") == 0) {
			int oI = 0;
			int result = 0;
			do {
				oI++;
			} while (token_table[i]->operand[oI][0] != '\0');

			code_table[index].format = oI;
			code_table[index].addr = sym_table[i].addr;
			code_table[index].record = 'D';
			code_table[index].index = i;
			index++;
		}
		else if (strcmp(token_table[i]->operator, "EXTREF") == 0) {
			memset(EXTREF, 0, sizeof(EXTREF));

			int oI = 0;
			// adding to EXTREF 
			for (oI = 0; token_table[i]->operand[oI][0] != '\0'; oI++)
				strcpy(EXTREF[oI], token_table[i]->operand[oI]);
			code_table[index].format = oI;
			code_table[index].record = 'R';
			code_table[index].index = i;
		}
		/// 
		//operator in inst.data
		if ((result = search_opcode(token_table[i]->operator)) > -1) {
			token_table[i]->nixbpe = 0;
			///complete ni
			//2byte (n=0, i=0)
			if (inst_table[result]->format == 2) {
				token_table[i]->nixbpe |= 0b000000; //n=0, i=0 (XXXX XX00 XXXX XXXX XXXX XXXX XXXX XXXX) X=completed, 0 = todo
			}
			//immediate (n=0, i=1)
			else if (token_table[i]->operand[0][0] == '#') {
				token_table[i]->nixbpe |= 0b010000; //n=0, i=1 (XXXX XX01 XXXX XXXX XXXX XXXX XXXX XXXX)
			}
			//indirect (n=1, i=0)
			else if (token_table[i]->operand[0][0] == '@') {
				token_table[i]->nixbpe |= 0b100000; //n = 1, i = 0 (XXXX XX10 XXXX XXXX XXXX XXXX XXXX XXXX)
			}
			//direct (n=1, i=1)
			else {
				token_table[i]->nixbpe |= 0b110000; //n=1, i=1 (XXXX XX11 XXXX XXXX XXXX XXXX XXXX XXXX)
			}

				//x
				if (strcmp(token_table[i]->operand[1], "X") == 0) {
					token_table[i]->nixbpe |= 0b001000; //x=1 (XXXX XXXX 1XXX XXXX XXXX XXXX XXXX XXXX)
				}
				//extended 
				if (token_table[i]->operator[0] == '+') {
					token_table[i]->nixbpe |= 0b000001; //e=1, (XXXX XXXX XXX1 XXXX XXXX XXXX XXXX XXXX)
					//code_table[index].code.d |= 0b0001; // e=1
				}
				for (int j = 0; j < MAX_OPERAND; j++) {
					if (strcmp(EXTREF[j], token_table[i]->operand[0]) == 0) {
						token_table[i]->nixbpe |= 0b000000; //all = (XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX)
					}
					//program counter
					else if (token_table[i]->operator[0] != '+') { 
						token_table[i]->nixbpe |= 0b000010; //p= 1; (XXXX XXXX XX1X XXXX XXXX XXXX XXXX XXXX)
					}
				}
				//immediate addressing
				if (token_table[i]->operand[0][0] == '#') {
					token_table[i]->nixbpe &= ~(1 << 1); //p= 0; (XXXX XXXX XX0X XXXX XXXX XXXX XXXX XXXX)
					//code_table[index].code.d &= ~(1 << 1); //xbpe p=0
				}


					///complete displacement & code
					code_table[index].format = inst_table[result]->format;
					//format 4
					if (token_table[i]->operator[0] == '+') {
						code_table[index].format++; //3 -> 4
					}
					code_table[index].code = inst_table[result]->op;
					code_table[index].record = 'T';
					code_table[index].index = i;
					code_table[index].addr = sym_table[i].addr;
					switch (code_table[index].format) {
					case 2: //2bytes //16bit, 4 in HEX
						code_table[index].code = code_table[index].code << 8; 
						for (int j = 0; token_table[i]->operand[j][0] != '\0' && j < 2; j++) {
							if (j == 0) {
								code_table[index].code = (inst_table[result]->op) << 8;
							}
							if ((strcmp(token_table[i]->operand[j], "A")) == 0)
								code_table[index].code |= 0x00 >> 4 * j;
							else if ((strcmp(token_table[i]->operand[j], "X")) == 0)
								code_table[index].code |= 0x10 >> 4 * j;
							else if ((strcmp(token_table[i]->operand[j], "L")) == 0)
								code_table[index].code |=  0x20 >> 4 * j;
							else if ((strcmp(token_table[i]->operand[j], "B")) == 0)
								code_table[index].code |= 0x30 >> 4 * j;
							else if ((strcmp(token_table[i]->operand[j], "S")) == 0)
								code_table[index].code |= 0x40>>4*j;
							else if ((strcmp(token_table[i]->operand[j], "T")) == 0)
								code_table[index].code |= 0x50 >> 4 * j;
							else if ((strcmp(token_table[i]->operand[j], "F")) == 0)
								code_table[index].code |= 0x60 >> 4 * j;
							else if ((strcmp(token_table[i]->operand[j], "PC")) == 0)
								code_table[index].code |= 0x80 >> 4 * j;
							else if ((strcmp(token_table[i]->operand[j], "SW")) == 0)
								code_table[index].code |= 0x90 >> 4 * j;
							else {
								code_table[index].code |= 0;
							}
					
						}
						break;
					case 3://3bytes //24bit, 6 in HEX
						code_table[index].code = code_table[index].code << 16; 
						//no operand
						if (inst_table[result]->ops == 0) {
							code_table[index].code |= ((token_table[i]->nixbpe) & 0b110000) << 12; // 
						}

						else {
							code_table[index].code |= (token_table[i]->nixbpe) << 12;

							//immediate addressing (#)
							if (token_table[i]->operand[0][0] == '#') {
								code_table[index].code  |= atoi(token_table[i]->operand[0] + 1);
							}

							else {
								//operand exist in symbol table
								if ((result = search_symbol(token_table[i]->operand[0], blockctr)) >= 0) {
									code_table[index].code |= sym_table[result].addr - sym_table[i + 1].addr & 0xFFF;
								}
								else {
									for (int j = 0; j < line_num; j++) {
										if (literal_table[j].literal != NULL && strcmp(literal_table[j].literal, token_table[i]->operand[0]) == 0) {
											code_table[index].code |= literal_table[j].addr - sym_table[i + 1].addr;
											break;
										}
									}
								}
							}
						}
						break;
					case 4://4bytes //32bit, 8 in HEX
						code_table[index].code = code_table[index].code << 24;
						code_table[index].code |= (token_table[i]->nixbpe) << 20;
						//immediate addressing (#)
						if (token_table[i]->operand[0][0] == '#') {
							code_table[index].code |= atoi(token_table[i]->operand[0]);
						}
						else {
							for (int j = 0; (j < MAX_OPERAND) && (EXTREF[j][0] != '\0'); j++) {
								if (strcmp(EXTREF[j], token_table[i]->operand[0]) == 0) {
									code_table[index].addr = sym_table[i].addr + 0b1;
									code_table[index].format = 4;
									code_table[index].record = 'M';
								}
							}
						}
						break;
					}
		}

		//operator NOT in inst.data RESW, RESB, BYTE, WORD, LTORG, END 
		else {
			if (strcmp(token_table[i]->operator, "RESW") == 0) {
				locctr += 3 * atoi(token_table[i]->operand[0]);
			}
			else if (strcmp(token_table[i]->operator, "RESB") == 0) {
				locctr +=  atoi(token_table[i]->operand[0]);
			}
			else if (strcmp(token_table[i]->operator, "BYTE") == 0) {
				code_table[index].record = 'T';
				code_table[index].index = i;
				
				tmp = (char*)malloc((strlen(token_table[i]->operand[0]) - 3 + 1) * sizeof(char));
				memset(tmp, 0, (strlen(token_table[i]->operand[0]) - 3 + 1) * sizeof(char));
				int k = 2;
				for (int l = 0; l < strlen(token_table[i]->operand[0]) - 3; l++) {
					tmp[l] = token_table[i]->operand[0][k + l];
				}

				if (token_table[i]->operand[0][0] == 'X') {
					int k = 0;
					for (int l = 0; tmp[l]!=NULL; l++) {
						code_table[index].code <<= 4;
						if ('A' <= tmp[l] && tmp[l] <= 'F')
							code_table[index].code |= (tmp[l] - 'A' + 10);
						else
							code_table[index].code |= (tmp[l] - '0');
					}
					code_table[index].format = (strlen(tmp)) / 2;
					code_table[index].addr = sym_table[i].addr;
				}

				else {
					locctr = strlen(token_table[token_line]->operand[0]) - 3;
					for (int i = 0; i < strlen(tmp); i++) {
						code_table[index].code = (code_table[index].code << 8) | tmp[i];
					}
					code_table[index].format = strlen(tmp);
					code_table[index].addr = locctr;
				}
				free(tmp);
			}
			else if (strcmp(token_table[i]->operator, "WORD") == 0) {
				//operand is character
				if (atoi(token_table[i]->operand[0]) == 0 && strlen(token_table[i]->operand[0]) > 1) {
					code_table[index].format = 3;
					code_table[index].addr += sym_table[i].addr;
					code_table[index].record = 'T';
					code_table[index].index = i;
					char* tmpOperand;
					tmpOperand = (char*)malloc((strlen(token_table[i]->operand[0]) + 1) * sizeof(char));
					memset(tmpOperand, 0, (strlen(token_table[i]->operand[0]) + 1) * sizeof(char));
					char* first;
					char* second;

					int first_operand;
					int second_operand;
					int operand_index = 0; //index for first symbol

					first = (char*)malloc((strlen(token_table[i]->operand[0]) + 1) * sizeof(char));
					memset(first, 0, (strlen(token_table[i]->operand[0]) + 1) * sizeof(char));
					strcpy(first, token_table[i]->operand[0]);


					for (int j = 0; first[j] != '-' || first[j] == NULL; j++) {
						operand_index++;
					}

					if (operand_index < strlen(token_table[i]->operand[0])) {
						operand_index++;
						second = (char*)malloc((strlen(first + operand_index) + 1) * sizeof(char));
						memset(second, 0, (strlen(first + operand_index) + 1) * sizeof(char));
						for (int j = 0; first[operand_index + j] != NULL; j++) {
							second[j] = first[operand_index + j];
						}
						first[operand_index - 1] = '\0';
						first_operand = search_symbol(first, blockctr);
						second_operand = search_symbol(second, blockctr);

						if (first_operand = -4 && second_operand == -4) {
							code_table[index].code = first_operand - second_operand;
						}
						else {//modification
							code_table[index].code = 0;
							index++;
							code_table[index].addr += sym_table[i].addr;
							code_table[index].format = 3; //length of the field to be modified
							code_table[index].record = 'M';
							code_table[index].index = i;
							index++;

							code_table[index].addr += sym_table[i].addr;
							code_table[index].format = 3; //length of the field to be modified
							code_table[index].record = 'M';
							code_table[index].index = i;
							index++;
						}
					}
					else {
						int first_operand = search_symbol(i, first);
						if (first_operand != -4) {
							code_table[index].code = sym_table[first_operand].addr;
						}
						else
							code_table[index].code = 0;
					}
					

					free(tmpOperand);
					free(first);
				}

				//operand is number
				else {
					code_table[index].format = 3;
					code_table[index].addr += sym_table[i].addr;
					code_table[index].record = 'T';
					code_table[index].index = i;
				}
			}
			else if (strcmp(token_table[i]->operator, "LTORG") == 0 ||
				(strcmp(token_table[i]->operator, "END")) == 0) {
				code_table[index].addr = sym_table[i].addr;
				code_table[index].record = 'T';
				code_table[index].index = i;

				for (int j = 0; j < line_num; j++)
					if (literal_table[j].addr == sym_table[i].addr) {
						tmp = (char*)malloc((strlen(literal_table[j].literal) - 4 + 1) * sizeof(char));
						memset(tmp, 0, (strlen(literal_table[j].literal) - 4 + 1) * sizeof(char));

						int k = 3;
						for (int l = 0; l < strlen(literal_table[j].literal) - 4; l++) {
							tmp[l] = literal_table[j].literal[k + l];
						}


						if (token_table[i]->operand[0][0] == 'X') {
							for (int l = 0; l < strlen(tmp); l++) {
								if ('A' <= tmp[l] && tmp[l] <= 'F')
									code_table[index].code = (code_table[index].code << 4) | (tmp[l] - 'A' + 10);
								else
									code_table[index].code = (code_table[index].code << 4) | (tmp[l] - '0');
							}
							code_table[index].format = (strlen(tmp)) / 2;
						}

						if (literal_table[j].literal[1] == 'X') {
							for (int l = 0; l < strlen(tmp); l++) {
								if ('A' <= tmp[l] && tmp[l] <= 'F')
									code_table[index].code = (code_table[index].code << 4) | (tmp[l] - 'A' + 10);
								else
									code_table[index].code = (code_table[index].code << 4) | (tmp[l] - '0');
							}
							code_table[index].format = (strlen(tmp)) / 2;
						}

						else {
							//code_table[index].code.e = tmp;
							for (int l = 0; l < strlen(tmp); l++) {
								code_table[index].code = (code_table[index].code << 8) | tmp[l];
							}
							code_table[index].format = strlen(tmp);
						}
						break;
						free(tmp);
					}

				length = code_table[index].format;
			}
		}
		if (sym_table[i].addr == E_addr) {
			code_table[H_index].addr += code_table[index].format;
		}
		index++;
	}
	code_table[index].format = 0;
	code_table[index].addr += code_table[H_index].format;
	code_table[index].record = 'E';
	code_table[index].index = ++i;
	index++;
	return 0;

}


/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 object code (프로젝트 1번) 이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*
* -----------------------------------------------------------------------------------
*/
void make_objectcode_output(char* file_name)
{

	/* add your code here */
	int i = 0, j = 0, k = 0;
	int H_index = 0;

	FILE* file;

	static int cover = 0;
	static int length = 0;
	static int area = 0;
	static int next = 0;
	static int last = 0; 

	static int modi_number = 0;
	static int prev = 0;

	char* ptr;
	ptr = 0;
	blockctr = 0;
	if (file_name == NULL) {
		file = stdout;//stdout
	}
	else {
		file = fopen(file_name, "w");
	}
	//file = stdout;
	for (i = 0; i < index; i++) {
		if (code_table[i].record == 'H') {
			fprintf(file, "H");
			fprintf(file, "%-6s", token_table[code_table[i].index]->label);
			fprintf(file, "%06X", 0);
			for (int j = i + 1; j < index; j++) {
				if (code_table[j].addr == code_table[i].addr) {
					code_table[i].addr += code_table[j].format;
				}
			}
			fprintf(file, "%06X", code_table[i].addr);
		}
		else if (code_table[i].record == 'D') {
			fprintf(file, "\n");
			fprintf(file, "D");
			for (j = 0; j < code_table[i].format; j++) {
				fprintf(file, "%-6s", token_table[code_table[i].index]->operand[j]);
				int k = 0;
				while (1) {
					if (sym_table[k].symbol != NULL && strcmp(sym_table[k].symbol, token_table[code_table[i].index]->operand[j]) == 0)
						break;
					k++;
				}
				fprintf(file, "%06X", sym_table[k].addr);
			}
		}
		else if (code_table[i].record == 'R') {
			fprintf(file, "\n");
			fprintf(file, "R");
			for (j = 0; j < code_table[i].format; j++) {
				fprintf(file, "%-6s", token_table[code_table[i].index]->operand[j]);
			}
		}
		else if (code_table[i].record == 'T') {
			if (cover--) {
				switch (code_table[i].format) {
				case 1:
					fprintf(file, "%.2X", code_table[i].code);
					break;
				case 2:
					fprintf(file, "%.4X", code_table[i].code);
					break;
				case 3:
					fprintf(file, "%.6X", code_table[i].code);
					break;
				case 4:
					fprintf(file, "%.8X", code_table[i].code);
					break;
				}
				//fprintf(file, "//");
			}
			else {
				cover = 0;
				next = 1;
				length = 0;
				prev == 0;
				do {
					if (code_table[i + next].addr - code_table[i + next].addr > 30)
						break;
					if ((length + code_table[i + next].format) > 30) { //
						break;
					}
					if ((token_table[code_table[i+next].index]->operator != NULL) && (strcmp (token_table[code_table[i + next].index]->operator, "LTORG")) == 0) { //
						break;
					}
					if (code_table[i+next].addr + code_table[i + next].format - code_table[i].addr > 30) {//if beyond cover exclude it
						next++;
						continue;
					}
					
					if (code_table[i + next].record == 'T' ||
						code_table[i + next].record == 'M') {
						length += code_table[i + next].format;
						cover++;
						prev = i + next;
					}
					
					next++;
				} while (code_table[i + next].record != 'E');

				int last_index = code_table[prev].index;
				int area = sym_table[last_index].addr + code_table[prev].format - code_table[i].addr;

				fprintf(file, "\n");
				fprintf(file, "T");
				fprintf(file, "%06X", code_table[i].addr);
				if (area > 0)
					fprintf(file, "%02X", area);
				else {//assume last_index is literal
					fprintf(file, "%02X", code_table[i + cover].addr + code_table[i + cover].format - code_table[i].addr);
				}
				//printed the first one
				switch (code_table[i].format) {
				case 1:
					fprintf(file, "%.2X", code_table[i].code);
					break;
				case 2:
					fprintf(file, "%.4X", code_table[i].code);
					break;
				case 3:
					fprintf(file, "%.6X", code_table[i].code);
					break;
				case 4:
					fprintf(file, "%.8X", code_table[i].code);
					break;
				}
				//fprintf(file, "//");

			}
			/*else {
				length = 0;
				cover = 1;
				subtract = 0;
				do {
					if ((length + code_table[i + cover].format) > 30) {
						break;
					}
					if (code_table[i + cover].record == 'T' ||
						code_table[i + cover].record == 'M') {
						length += code_table[i + cover].format;
						cover++;
					}
					else { cover++; subtract++; continue; }

				} while (code_table[i + cover].record != 'E');

				cover -= subtract + 1;
				fprintf(file, "\n");
				fprintf(file, "%c", code_table[i].record);
				fprintf(file, "%06X//", code_table[i].addr);

				int area = 0;
				area = code_table[i + cover].addr + code_table[i + cover].format - code_table[i].addr;
				if (area > 0)
					fprintf(file, "%02X", area);
				else {
					fprintf(file, "%02X", length);
				}
				fprintf(file, "%X//", code_table[i].code);
			}*/
		}
		else if (code_table[i].record == 'M') {
			static int prev = 0;

			if (code_table[i].index == prev) {
				cover--;
			}
			else if (code_table[i].index == code_table[i + 1].index) {
				cover--;
			}
			else if (cover) {
				fprintf(file, "%X", code_table[i].code);
				cover--;
			}
			modi_number++;
			prev = code_table[i].index;
			
		}
		else if (code_table[i].record == 'E') {
			for (j = modi_index; j < index; j++) {
				if (code_table[j].record != NULL && code_table[j].record == 'M') {
					fprintf(file, "\nM");

					switch (code_table[j].format) {
					case 4:
						fprintf(file, "%06X", sym_table[code_table[j].index].addr+1);
						fprintf(file, "%02X", 5);
						break;
					case 3:
						fprintf(file, "%06X", sym_table[code_table[j].index].addr);
						fprintf(file, "%02X", 6);
						break;
					}
					if (token_table[code_table[j].index]->operand[0] != NULL) {
						if (ptr) {
							fprintf(file, "-");
							do {
								fprintf(file, "%c", *(ptr++));
							} while (ptr[0] != NULL);
							ptr = 0;
						}
						else {
							k = 0;
							fprintf(file, "+");
							while (k < strlen(token_table[code_table[j].index]->operand[0])) {
								if (token_table[code_table[j].index]->operand[0][k] == '-') {
									ptr = &token_table[code_table[j].index]->operand[0][k + 1];
									break;
								}
								if (token_table[code_table[j].index]->operand[0][k] == '\0') {
									break;
								}
								fprintf(file, "%c", token_table[code_table[j].index]->operand[0][k]);
								k++;
							}
						}
						modi_number--;
						if (modi_number == 0) {
							break;
						}
					}
				}
				blockctr;
			}
			modi_index = j + 1;
			blockctr++;
			fprintf(file, "\n");
			if (blockctr == 1)
				fprintf(file, "E%06X\n\n", 0);
			else
				fprintf(file, "E\n\n");
		}

	}
	fprintf(file, "\n");
	fclose(file);
}
