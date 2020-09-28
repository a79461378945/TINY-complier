#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

enum TOKEN_TYPE { KEY, SYM, ID, NUM, STR ,EMPTY};

struct Token {
    TOKEN_TYPE type;
    string value;
    Token(TOKEN_TYPE t, string val) {
        type = t;
        value = val;
    }
    Token() {
        type = EMPTY;
        value = "";
    }
    void print() {
        switch (type)
        {
        case KEY:
            cout << "(KEY," << value << ")";
            break;
        case SYM:
            cout << "(SYM," << value << ")";
            break;
        case ID:
            cout << "(ID," << value << ")";
            break;
        case NUM:
            cout << "(NUM," << value << ")";
            break;
        case STR:
            cout << "(STR," << value << ")";
            break;
        default:
            cout << "(-1," << value << ")";
            break;
        }
    }
};
vector<Token> tokens;
//15
string keywords[]={ "if","then","else","end","repeat","until",
"read","write","or","and","int","bool","char","while","do" };
//15
string symbols[]={ "+","-","*","/","=","<","(",")",";",
":=",">","<=",">=",",","'" };
int next_state = 0;
int line_cnt = 1;//��ǵ�ǰɨ�赽������
string buf = "";//��ǰɨ���Ļ�û��token���ַ�����
int sym_used = 1;//��ʶcursym�Ѿ��ù�����ù�Ϊ1������Ϊ0  
char cursym = ' ';
void error(string mes) {
    cout << "error at line " << line_cnt << " :" << mes << endl;
}

int S0_initial(FILE* source) {//��ʼ̬ ����ֵ��ʾ�Ѿ�ʶ�������token���,-1��ʾ��ûʶ���
    if (sym_used == 1)cursym = fgetc(source);//��ǰ�����ù��˲��ٶ�
    cout<<"S0get:"<<int(cursym)<<endl;
    sym_used = 0;
    if (cursym == -1) {//EOF�Ѿ������� Ҫ���Զ�������cursym
        return -1;
    }
    if (cursym == ' ' || cursym == '\t') {//���ַ��ͼ����ص�0״̬
    	sym_used=1;
        next_state = 0;
    }
    else if (cursym == '\n') {
    	sym_used=1;
        next_state = 0;
        line_cnt += 1;
    }
    else if (isdigit(cursym)) {//������ �ǾͿ�����NUM��ȥS1ʶ��num
        next_state = 1;
    }
    else if (isalpha(cursym)) {//����ĸ��������ID��KEY��ȥS2ʶ��
        next_state = 2;
    }
    else if (cursym == '\'') {//�� ' �ǿ�����STR��ȥS3ʶ��
        next_state = 3;
    }
    else if (cursym != '{') {//�ֲ���'{' ��ֻ������SYM����ȥS4
        next_state = 4;
    }
    else {//cursym=='{' �⿪ʼ��ע��,ȥ��ע�Ͷ�ɨ���ٻ�����
        next_state = 5;
    }

    return -1;
}

int S1_num(FILE* source) {//ʶ��������
    buf += cursym;
    cursym = fgetc(source);
    if (cursym == -1) {//EOF �Ѿ�������
        return NUM;
    }
    if (cursym == ' ' || cursym == '\t') {//���ַ���ʶ��������
        next_state = 0;
        sym_used = 1;
        return NUM;
    }
    else if (cursym == '\n') {
        next_state = 0;
        line_cnt += 1;
        sym_used = 1;
        return NUM;
    }
    else if (isdigit(cursym)) {//�������־ͼ���ʶ�𣬼���S1
        next_state = 1;
    }
    else {//������Ҳ����ʶ�������������Ѿ�������һ��token�ˣ�
        next_state = 0;
        sym_used = 0;//������һ��token���ַ�����˱��һ��
        return NUM;
    }
    return -1;
}

int S2_id(FILE* source) {//��ĸ��ͷ�ģ���ʶ��û���� ɨ���ٿ��ǲ��ǹؼ���
    buf += cursym;
    cursym = fgetc(source);
    if (cursym == -1) {//EOF �Ѿ�������
        for (int i = 0; i < 15; i++) {//�����ǲ���keyword
            if (buf == keywords[i]) {
                return KEY;
            }
        }
        //����KEY�Ǿ���ID��
        return ID;
    }
    if (isdigit(cursym) || isalpha(cursym)) {//������ĸ�����֣��Ǿͼ���S2ʶ��
        next_state = 2;
    }
    else {//ʣ�µ�Ҫô�Ƿ���Ҫô�ǿո񣬷�������token��
        sym_used = 0;
        next_state = 0;
        for (int i = 0; i < 15; i++) {//�����ǲ���keyword
            if (buf == keywords[i]) {
                return KEY;
            }
        }
        //����KEY�Ǿ���ID��
        return ID;
    }
    return -1;

}

int S3_str(FILE* source) {// '��ͷ��ֻ�����ַ����� һֱʶ����һ��'Ϊֹ
    cursym = fgetc(source);//����һ�µ�һ���ַ�,ԭ����ͷ��'����Ҫ
    while (cursym != '\'') {
        if (cursym == -1) {//�ַ�����û�����ļ���û��
            error("�ַ�����û������û��");
            exit(0);
        }
        else if(cursym=='\n'){
        	error("�ַ������ܿ��ж��壡");
        	exit(0);
        }else{
        	buf += cursym;
		}
        cursym = fgetc(source);
    }
    //ɨ��cursym=='\''�� �ַ����Ѿ������
    sym_used = 1;//�ǵ����ù����'\''��
    next_state = 0;
    return STR;//ʶ��ɹ���
}


// vector<string> symbols(string[]{"+","-","*","/","=","<","(",")",";",
// ":=",">","<=",">=",",","'"},15);
int S4_sym(FILE* source) {
    buf += cursym;
    sym_used=1;//Ҫ������Ŷ� 
    //ע���ƥ��ԭ��
    if (cursym == ':' || cursym == '<' || cursym == '>') {//�п����������ַ���sym
        cursym = fgetc(source);
        if (cursym == '=') {//��ȷ�����ַ���
            buf += cursym;
            sym_used = 1;
        }
        else {//���ǣ��Ǿ����ô��ˣ������
            sym_used = 0;
        }
    }
    for (int i = 0; i < 15; i++) {
        if (buf == symbols[i]) {//ƥ���ϣ���ȷ�ǺϷ�SYM��
            next_state = 0;
            return SYM;
        }
    }
    //��ûƥ���ϣ�ʧ����
    error("��SYMBOL���԰���");
    exit(0);
    return -1;

}

int S5_note(FILE* source) {//��ע��ȫ������
    cursym = fgetc(source);//����һ�µ�һ���ַ�,ԭ����ͷ��'����Ҫ
    while (cursym != '}') {
        if (cursym == -1) {//ע�ͻ�û�����ļ���û��
            error("ע�ͻ�û������û��");
            exit(0);
        }
        if (cursym == '\n') {//ע���������������Ҫ���
            line_cnt++;
        }
        cursym = fgetc(source);
    }
    //ɨ��cursym=='\''�� �ַ����Ѿ������
    sym_used = 1;//�ǵ����ù����'}'��
    next_state = 0;
    return -1;//ע���Ѿ����꿩
}
int (*state[])(FILE* source) = { S0_initial,S1_num,S2_id,S3_str,S4_sym,S5_note };
Token getNextToken(FILE* source) {
    next_state = 0;
    int type = -1;
    buf = "";
    while (type = -1) {
    	cout<<"next_state:"<<next_state<<endl;
        type = state[next_state](source);
        if (type != -1) {
            return Token((TOKEN_TYPE)type, buf);
        }
        if(cursym==-1){
        	return Token();
		}
    }
}

int main() {
	char src_name[100];
	printf("please input the source file name:");
	scanf("%s",src_name);
    FILE* source = fopen(src_name, "r");
    while (cursym != -1) {
    	Token a=getNextToken(source);
    	a.print();
    	cout<<" ";
        tokens.push_back(a);
    }
	if(tokens[tokens.size()-1].type!=-1){
		tokens.push_back(Token());
	}
	
    fclose(source);
    cout<<endl<<endl<<"analyse result:"<<endl;
    for (int i = 0; i < tokens.size(); i++) {
        tokens[i].print();
        cout <<"  ";
        if(i%5==4)cout<<endl;
    }
}

