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
int line_cnt = 1;//标记当前扫描到的行数
string buf = "";//当前扫过的还没成token的字符序列
int sym_used = 1;//标识cursym已经用过与否，用过为1，否则为0  
char cursym = ' ';
void error(string mes) {
    cout << "error at line " << line_cnt << " :" << mes << endl;
}

int S0_initial(FILE* source) {//初始态 返回值表示已经识别出来的token类别,-1表示还没识别好
    if (sym_used == 1)cursym = fgetc(source);//当前符号用过了才再读
    cout<<"S0get:"<<int(cursym)<<endl;
    sym_used = 0;
    if (cursym == -1) {//EOF已经读完了 要在自动机里检测cursym
        return -1;
    }
    if (cursym == ' ' || cursym == '\t') {//空字符就继续回到0状态
    	sym_used=1;
        next_state = 0;
    }
    else if (cursym == '\n') {
    	sym_used=1;
        next_state = 0;
        line_cnt += 1;
    }
    else if (isdigit(cursym)) {//是数字 那就可能是NUM，去S1识别num
        next_state = 1;
    }
    else if (isalpha(cursym)) {//是字母，可能是ID或KEY，去S2识别
        next_state = 2;
    }
    else if (cursym == '\'') {//是 ' 那可能是STR，去S3识别
        next_state = 3;
    }
    else if (cursym != '{') {//又不是'{' 那只可能是SYM啦，去S4
        next_state = 4;
    }
    else {//cursym=='{' 这开始是注释,去把注释都扫完再回来！
        next_state = 5;
    }

    return -1;
}

int S1_num(FILE* source) {//识别数字中
    buf += cursym;
    cursym = fgetc(source);
    if (cursym == -1) {//EOF 已经读完了
        return NUM;
    }
    if (cursym == ' ' || cursym == '\t') {//空字符就识别完啦，
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
    else if (isdigit(cursym)) {//还是数字就继续识别，继续S1
        next_state = 1;
    }
    else {//其他那也当作识别完啦，可能已经遇到下一个token了，
        next_state = 0;
        sym_used = 0;//读了下一个token的字符，因此标记一下
        return NUM;
    }
    return -1;
}

int S2_id(FILE* source) {//字母开头的，标识符没跑啦 扫完再看是不是关键字
    buf += cursym;
    cursym = fgetc(source);
    if (cursym == -1) {//EOF 已经读完了
        for (int i = 0; i < 15; i++) {//看看是不是keyword
            if (buf == keywords[i]) {
                return KEY;
            }
        }
        //不是KEY那就是ID了
        return ID;
    }
    if (isdigit(cursym) || isalpha(cursym)) {//还是字母或数字，那就继续S2识别
        next_state = 2;
    }
    else {//剩下的要么是符号要么是空格，反正不是token的
        sym_used = 0;
        next_state = 0;
        for (int i = 0; i < 15; i++) {//看看是不是keyword
            if (buf == keywords[i]) {
                return KEY;
            }
        }
        //不是KEY那就是ID了
        return ID;
    }
    return -1;

}

int S3_str(FILE* source) {// '开头的只能是字符串咯 一直识别到另一个'为止
    cursym = fgetc(source);//先拿一下第一个字符,原来开头的'不需要
    while (cursym != '\'') {
        if (cursym == -1) {//字符串还没结束文件就没了
            error("字符串还没结束就没了");
            exit(0);
        }
        else if(cursym=='\n'){
        	error("字符串不能跨行定义！");
        	exit(0);
        }else{
        	buf += cursym;
		}
        cursym = fgetc(source);
    }
    //扫到cursym=='\''了 字符串已经完结了
    sym_used = 1;//记得是用过这个'\''了
    next_state = 0;
    return STR;//识别成功咯
}


// vector<string> symbols(string[]{"+","-","*","/","=","<","(",")",";",
// ":=",">","<=",">=",",","'"},15);
int S4_sym(FILE* source) {
    buf += cursym;
    sym_used=1;//要放在这才对 
    //注意最长匹配原则
    if (cursym == ':' || cursym == '<' || cursym == '>') {//有可能是两个字符的sym
        cursym = fgetc(source);
        if (cursym == '=') {//的确是两字符的
            buf += cursym;
            sym_used = 1;
        }
        else {//不是，那就是拿错了，标记下
            sym_used = 0;
        }
    }
    for (int i = 0; i < 15; i++) {
        if (buf == symbols[i]) {//匹配上，的确是合法SYM了
            next_state = 0;
            return SYM;
        }
    }
    //都没匹配上，失败啦
    error("这SYMBOL不对啊！");
    exit(0);
    return -1;

}

int S5_note(FILE* source) {//把注释全部过掉
    cursym = fgetc(source);//先拿一下第一个字符,原来开头的'不需要
    while (cursym != '}') {
        if (cursym == -1) {//注释还没结束文件就没了
            error("注释还没结束就没了");
            exit(0);
        }
        if (cursym == '\n') {//注释里面的行数还是要算的
            line_cnt++;
        }
        cursym = fgetc(source);
    }
    //扫到cursym=='\''了 字符串已经完结了
    sym_used = 1;//记得是用过这个'}'了
    next_state = 0;
    return -1;//注释已经过完咯
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

