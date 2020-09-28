#include "lexical_dev.cpp"
//首先定义文法符号 非终结符
enum syntax_symbols{program,declar,decl,type,varlist,
stmt_seq,stmt,while_stmt,if_stmt,repeat_stmt,assign_stmt,read_stmt,write_stmt,
exp,arith_exp,addop,term,mulop,factor,
bool_exp,bterm,bfactor,com_exp,com_op,str_exp};
string syntax_symbols_strs[]={"program","declar","decl","type","varlist",
"stmt_seq","stmt","while_stmt","if_stmt","repeat_stmt","assign_stmt","read_stmt","write_stmt",
"exp","arith_exp","addop","term","mulop","factor",
"bool_exp","bterm","bfactor","com_exp","com_op","str_exp"};
//结果addop,bfactor,com_op没用
int token_cnt=0;//计下当前处理到的token号
vector<int> tokens_line;//记一下这个token对应行数
int parsing_depth;

//定义语法树节点
struct Treenode{
    syntax_symbols symbol;
    vector<Treenode> subtree;
    Token token;//当这个是叶子节点时就是token了
    void print(int dep);
    int line;//节点所在行数
};
Treenode parse_program();
Treenode parse_declar();
Treenode parse_decl();
Treenode parse_type();
Treenode parse_varlist();
Treenode parse_stmt_seq();
Treenode parse_stmt();
Treenode parse_while_stmt();
Treenode parse_if_stmt();
Treenode parse_repeat_stmt();
Treenode parse_assign_stmt();
Treenode parse_read_stmt();
Treenode parse_write_stmt();
Treenode parse_exp();
Treenode parse_arith_exp();
Treenode parse_term();
Treenode parse_factor();
Treenode parse_bool_exp();
Treenode parse_bterm();
Treenode parse_bfactor();
Treenode parse_com_exp();


void Treenode::print(int dep){
    for(int i=0;i<dep;i++){
    	if(i%4==3)printf("|");
    	else printf(" ");
    }
    if(subtree.size()==0){//叶子节点
        token.print();
        printf("\n");
    }else{
        cout<<syntax_symbols_strs[symbol]<<endl;
        for(int i=0;i<subtree.size();i++){
            subtree[i].print(dep+4);
        }
    }
}

//先直接得到整个token序列再说
void prepare(FILE* source){
    // FILE* source = fopen("src.txt", "r");
    while (cursym != -1) {
    	Token a=getNextToken(source);
    	// a.print();
    	// cout<<" ";
        tokens.push_back(a);
        tokens_line.push_back(line_cnt);
    }
	if(tokens[tokens.size()-1].type!=-1){
		tokens.push_back(Token());
        tokens_line.push_back(line_cnt);
	}
}

void syntax_error(string expect,Token t){
    cout<<"syntax error at line "<<tokens_line[token_cnt]<<": expect "<<expect<<", but got a ";
    t.print();
    cout<<endl;
}

void parsing_mes_in(string mes){//用来在分析过程输出，方便知道分析过程 
//	for(int i=0;i<parsing_depth;i++){
//		if(i%4==3)printf("|");
//		else printf(" ");
//	}
//	cout<<mes<<" in ";
//	tokens[token_cnt].print();
//	cout<<endl;
	parsing_depth+=4;
}
void parsing_mes_out(string mes){//用来在分析过程输出，方便知道分析过程 
	parsing_depth-=4;
//	for(int i=0;i<parsing_depth;i++){
//		if(i%4==3)printf("|");
//		else printf(" ");
//	}
//	cout<<mes<<" out ";
//	tokens[token_cnt].print();
//	cout<<endl;
}
//终结符号与token的匹配
//终结符号有 各类符号，关键字，ID，string ，NUM
bool match(Token t,string t_sym){
    if(t_sym=="identifier"){
        return t.type==ID;
    }
    if(t_sym=="string"){
        return t.type==STR;
    }
    if(t_sym=="number"){
        return t.type==NUM;
    }
    if(t_sym=="$"){
        return t.type==EMPTY;
    }
    for(int i=0;i<15;i++){//看是否要匹配关键字
        if(t_sym==keywords[i]){
            if(t.type==KEY && t.value==t_sym){
                return true;
            }else{
                return false;
            }
        }
    }
    for(int i=0;i<15;i++){//看是否要匹配符号
        if(t_sym==symbols[i]){
            if(t.type==SYM && t.value==t_sym){
                return true;
            }else{
                return false;
            }
        }
    }
    //正常程序不管输入源码如何都应该是不会到这的
    cout<<"program bug:there is no "<<t_sym<<endl;
    exit(0);
}

//然后可以开始了

//first declarations可能为空、int、bool、char
//first stmt-sequence可能为while,if,repeat,identifier,read,write

Treenode parse_program(){//程序 对应规则1
    parsing_mes_in("parse_program");
    Treenode retnode;
    retnode.symbol=program;
    retnode.line=tokens_line[token_cnt];
    
    Treenode declarations=parse_declar();
    retnode.subtree.push_back(declarations);
    
    Treenode stmt_sequence=parse_stmt_seq();
    retnode.subtree.push_back(stmt_sequence);
    
    parsing_mes_out("parse_program");
    return retnode;
}
Treenode parse_declar(){//声明部分语句  对应规则2
	parsing_mes_in("parse_declar");
    Treenode retnode;
    retnode.symbol=declar;
    retnode.line=tokens_line[token_cnt];

    while(match(tokens[token_cnt],"int")||match(tokens[token_cnt],"bool")||match(tokens[token_cnt],"char")){
        retnode.subtree.push_back(parse_decl());
        if(!match(tokens[token_cnt],";")){//声明语句完了居然没有; 语法错
            syntax_error(";",tokens[token_cnt]);
        }else{//有那加入然后就下一个
            Treenode leaf;
            leaf.token=tokens[token_cnt];
            token_cnt++;
            retnode.subtree.push_back(leaf); 
        }
    }
    parsing_mes_out("parse_declar");
    return retnode;
}

Treenode parse_stmt_seq(){//语句序列 对应规则6
	parsing_mes_in("parse_stmt_seq");
    Treenode retnode;
    retnode.symbol=stmt_seq;
    retnode.line=tokens_line[token_cnt];

    int mark=0;//至少得匹配一个
    while(1){
        Treenode t1;
        t1=parse_stmt();
        retnode.subtree.push_back(t1);
        if(match(tokens[token_cnt],";")){//有;说明还有得匹配
            Treenode leaf;
            leaf.token=tokens[token_cnt];
            token_cnt++;
            retnode.subtree.push_back(leaf);
        }else{//没有逗号了那就是正常完了，可以回去了
        	parsing_mes_out("parse_stmt_seq");
            return retnode;
        }
    }

}

Treenode parse_decl(){//单个声明语句 对应规则3
    //之前parse_declar已经确定开头是specifier了
    parsing_mes_in("parse_decl");
    Treenode retnode;
    retnode.symbol=decl;
    retnode.line=tokens_line[token_cnt];
    Treenode s1=parse_type();
    retnode.subtree.push_back(s1);
    Treenode s2=parse_varlist();
    retnode.subtree.push_back(s2);
    parsing_mes_out("parse_decl");
    return retnode;
}
Treenode parse_type(){//类型标识符 对应规则4
	parsing_mes_in("parse_type");
    Treenode retnode;
    retnode.symbol=type;
    retnode.line=tokens_line[token_cnt];
    string strs[]={"int","bool","char"};
    for(int i=0;i<3;i++){
        if(match(tokens[token_cnt],strs[i])){
            Treenode leaf;
            leaf.token=tokens[token_cnt];
            token_cnt++;
            retnode.subtree.push_back(leaf);
            parsing_mes_out("parse_type");
            return retnode;
        }
    }
    //没匹配上，理论上也不会到这的
    syntax_error("type-specifier",tokens[token_cnt]);
    exit(0);
}
Treenode parse_varlist(){//变量标识符列表 对应规则5
	parsing_mes_in("parse_varlist");
    Treenode retnode;
    retnode.symbol=varlist;
    retnode.line=tokens_line[token_cnt];
    int mark=0;//至少得匹配一个
    while(match(tokens[token_cnt],"identifier")){
        mark=1;
        Treenode leaf1;
        leaf1.token=tokens[token_cnt];
        token_cnt++;
        retnode.subtree.push_back(leaf1);
        if(match(tokens[token_cnt],",")){//有逗号说明还有得匹配
            Treenode leaf2;
            leaf2.token=tokens[token_cnt];
            token_cnt++;
            retnode.subtree.push_back(leaf2);
        }else{//没有逗号了那就是正常完了，可以回去了
        	parsing_mes_out("parse_varlist");
            return retnode;
        }
    }
    // 没有接上标识符，语法错误了
    syntax_error("identifier",tokens[token_cnt]);
    exit(0);
}

Treenode parse_stmt(){// 对应规则7 
	parsing_mes_in("parse_stmt");
    Treenode retnode;
    retnode.symbol=stmt;
    retnode.line=tokens_line[token_cnt];
    Treenode t1;
    if(match(tokens[token_cnt],"while")){
        t1=parse_while_stmt();
    }else if(match(tokens[token_cnt],"if")){
        t1=parse_if_stmt();
    }else if(match(tokens[token_cnt],"repeat")){
        t1=parse_repeat_stmt();
    }else if(match(tokens[token_cnt],"identifier")){
        t1=parse_assign_stmt();
    }else if(match(tokens[token_cnt],"read")){
        t1=parse_read_stmt();
    }else if(match(tokens[token_cnt],"write")){
        t1=parse_write_stmt();
    }else{
        syntax_error("statement",tokens[token_cnt]);
        exit(0);
    }
    retnode.subtree.push_back(t1);
    parsing_mes_out("parse_stmt");
    return retnode;
}

Treenode parse_while_stmt(){//规则8
	parsing_mes_in("parse_while_stmt");
    Treenode retnode;
    retnode.symbol=while_stmt;
    retnode.line=tokens_line[token_cnt];

    Treenode leaf1;//进来之前已经知道当前token是while了
    leaf1.token=tokens[token_cnt];
    token_cnt+=1;
    retnode.subtree.push_back(leaf1);

    Treenode t1;
    t1=parse_bool_exp();
    retnode.subtree.push_back(t1);

    if(match(tokens[token_cnt],"do")){
        Treenode leaf2;
        leaf2.token=tokens[token_cnt];
        token_cnt+=1;
        retnode.subtree.push_back(leaf2);
    }else{
        syntax_error("keyword 'do'",tokens[token_cnt]);
        exit(0);
    }

    Treenode t2;
    t2=parse_stmt_seq();
    retnode.subtree.push_back(t2);

    if(match(tokens[token_cnt],"end")){
        Treenode leaf3;
        leaf3.token=tokens[token_cnt];
        token_cnt+=1;
        retnode.subtree.push_back(leaf3);
    }else{
        syntax_error("keyword 'end'",tokens[token_cnt]);
        exit(0);
    }
    parsing_mes_out("parse_while_stmt");
    return retnode;
}
Treenode parse_if_stmt(){//规则9
	parsing_mes_in("parse_if_stmt");
    Treenode retnode;
    retnode.symbol=if_stmt;
    retnode.line=tokens_line[token_cnt];

    Treenode leaf1;//进来之前已经知道当前token是if了
    leaf1.token=tokens[token_cnt];
    token_cnt+=1;
    retnode.subtree.push_back(leaf1);

    Treenode t1;
    t1=parse_bool_exp();
    retnode.subtree.push_back(t1);

    if(match(tokens[token_cnt],"then")){
        Treenode leaf2;
        leaf2.token=tokens[token_cnt];
        token_cnt+=1;
        retnode.subtree.push_back(leaf2);
    }else{
        syntax_error("keyword 'then'",tokens[token_cnt]);
        exit(0);
    }

    Treenode t2;
    t2=parse_stmt_seq();
    retnode.subtree.push_back(t2);

    if(match(tokens[token_cnt],"else")){//还有else 的话
        Treenode leaf_else;
        leaf_else.token=tokens[token_cnt];
        token_cnt+=1;
        retnode.subtree.push_back(leaf_else);

        Treenode t3;
        t3=parse_stmt_seq();
        retnode.subtree.push_back(t3);
    }

    if(match(tokens[token_cnt],"end")){
        Treenode leaf3;
        leaf3.token=tokens[token_cnt];
        token_cnt+=1;
        retnode.subtree.push_back(leaf3);
    }else{
        syntax_error("keyword 'end'",tokens[token_cnt]);
        exit(0);
    }
    parsing_mes_out("parse_if_stmt");
    return retnode;
}
Treenode parse_repeat_stmt(){//规则10
	parsing_mes_in("parse_repeat_stmt");
    Treenode retnode;
    retnode.symbol=repeat_stmt;
    retnode.line=tokens_line[token_cnt];

    Treenode leaf1;//进来之前已经知道当前token是repeat了
    leaf1.token=tokens[token_cnt];
    token_cnt+=1;
    retnode.subtree.push_back(leaf1);

    Treenode t1;
    t1=parse_stmt_seq();
    retnode.subtree.push_back(t1);

    if(match(tokens[token_cnt],"until")){
        Treenode leaf2;
        leaf2.token=tokens[token_cnt];
        token_cnt+=1;
        retnode.subtree.push_back(leaf2);
    }else{
        syntax_error("keyword 'until'",tokens[token_cnt]);
        exit(0);
    }

    Treenode t2;
    t2=parse_bool_exp();
    retnode.subtree.push_back(t2);
	parsing_mes_out("parse_repeat_stmt");
    return retnode;
}
Treenode parse_assign_stmt(){//规则11
	parsing_mes_in("parse_assign_stmt");
    Treenode retnode;
    retnode.symbol=assign_stmt;
    retnode.line=tokens_line[token_cnt];

    Treenode leaf1;//进来之前已经知道当前token是ID了
    leaf1.token=tokens[token_cnt];
    token_cnt+=1;
    retnode.subtree.push_back(leaf1);

    if(match(tokens[token_cnt],":=")){
        Treenode leaf2;
        leaf2.token=tokens[token_cnt];
        token_cnt+=1;
        retnode.subtree.push_back(leaf2);
    }else{
        syntax_error("symbol ':='",tokens[token_cnt]);
        exit(0);
    }

    Treenode t1;
    t1=parse_exp();
    retnode.subtree.push_back(t1);
	parsing_mes_out("parse_assign_stmt");
    return retnode;
}
Treenode parse_read_stmt(){//规则12
	parsing_mes_in("parse_read_stmt");
    Treenode retnode;
    retnode.symbol=read_stmt;
    retnode.line=tokens_line[token_cnt];

    Treenode leaf1;//进来之前已经知道当前token是read了
    leaf1.token=tokens[token_cnt];
    token_cnt+=1;
    retnode.subtree.push_back(leaf1);

    if(match(tokens[token_cnt],"identifier")){
        Treenode leaf2;
        leaf2.token=tokens[token_cnt];
        token_cnt+=1;
        retnode.subtree.push_back(leaf2);
    }else{
        syntax_error("identifier",tokens[token_cnt]);
        exit(0);
    }
    parsing_mes_out("parse_read_stmt");
    return retnode;
}
Treenode parse_write_stmt(){//规则13
	parsing_mes_in("parse_write_stmt");
    Treenode retnode;
    retnode.symbol=write_stmt;
    retnode.line=tokens_line[token_cnt];

    Treenode leaf1;//进来之前已经知道当前token是write了
    leaf1.token=tokens[token_cnt];
    token_cnt+=1;
    retnode.subtree.push_back(leaf1);

    Treenode t1;
    t1=parse_exp();
    retnode.subtree.push_back(t1);
	parsing_mes_out("parse_write_stmt");
    return retnode;
}


Treenode parse_exp(){//规则14
	parsing_mes_in("parse_exp");
    Treenode retnode;
    retnode.symbol=exp;
    retnode.line=tokens_line[token_cnt];
    Treenode t1;
    if(match(tokens[token_cnt],"string")){//是str那就直接好判断
        t1.token=tokens[token_cnt];
        token_cnt += 1;
    }else{//由于bool-exp也可能以arithmetic-exp开头，所以这里还得先判断
        //也可以通过更改文法定义的方式来做，但这里就直接简单做法好了
        //先找到follow(exp)=follow(write-stmt)+follow(assign-stmt)=follow(statement)
        // follow(statement)= ';' + follow(stmt-sequence) 
        //  = ';' + '$' + "end" + "else" + "until"
        string follow_exp[]={";","$","end","else","until"};
        string comops[]={"<","=",">","<=",">="};
        int i=1;//从当前之后第一个开始找，因为arithmetic-exp的first不为空
        int bool_mark=0;

        while(1){
            int end_mark=0;
            for(int j=0;j<5;j++){//是否已经遇到exo之后了
                if(match(tokens[token_cnt+i],follow_exp[j])){
                    end_mark=1;
                }
            }
            if(end_mark==1)break;
            for(int j=0;j<5;j++){//找下有没有比较运算符
                if(match(tokens[token_cnt+i],comops[j])){
                    bool_mark=1;
                    end_mark=1;
                    break;
                }
            }
            if(end_mark==1)break;
            i++;
            if(i>=tokens.size()){//正常程序最后一个token是结束符应该不会执行到这里的
                cout<<"program bug,exceed tokens' size"<<endl;
                exit(0);
            }
        }
        //现在已经知道该选择哪个表达式了
        if(bool_mark==1){
            t1=parse_bool_exp();
        }else{
            t1=parse_arith_exp();
        }
    }
    retnode.subtree.push_back(t1);
    parsing_mes_out("parse_exp");
    return retnode;

}

Treenode parse_arith_exp(){//规则15
	parsing_mes_in("parse_arith_exp");
    Treenode retnode;
    retnode.symbol=arith_exp;
    retnode.line=tokens_line[token_cnt];

    while(1){
        Treenode t1;
        t1=parse_term();
        retnode.subtree.push_back(t1);
        //规则16直接替换过来了不另外写函数了
        if(match(tokens[token_cnt],"+")||match(tokens[token_cnt],"-")){//有;说明还有得匹配
            Treenode leaf;
            leaf.token=tokens[token_cnt];
            token_cnt++;
            retnode.subtree.push_back(leaf);
        }else{//没有+-那就是正常完了，可以回去了
        	parsing_mes_out("parse_arith_exp");
            return retnode;
        }
    }
}
Treenode parse_term(){//规则17
	parsing_mes_in("parse_term");
    Treenode retnode;
    retnode.symbol=term;
    retnode.line=tokens_line[token_cnt];
    while(1){
        Treenode t1;
        t1=parse_factor();
        retnode.subtree.push_back(t1);
        //规则18直接替换过来了不另外写函数了
        if(match(tokens[token_cnt],"*")||match(tokens[token_cnt],"/")){//有;说明还有得匹配
            Treenode leaf;
            leaf.token=tokens[token_cnt];
            token_cnt++;
            retnode.subtree.push_back(leaf);
        }else{//没有* / 那就是正常完了，可以回去了
        	parsing_mes_out("parse_term");
            return retnode;
        }
    }
}
Treenode parse_factor(){//规则19
	parsing_mes_in("parse_factor");
    Treenode retnode;
    retnode.symbol=factor;
    retnode.line=tokens_line[token_cnt];
    if(match(tokens[token_cnt],"(")){
        Treenode leaf1;
        leaf1.token=tokens[token_cnt];
        token_cnt+=1;
        retnode.subtree.push_back(leaf1);

        Treenode t1;
        t1=parse_arith_exp();
        retnode.subtree.push_back(t1);

        if(match(tokens[token_cnt],")")){
            Treenode leaf2;
            leaf2.token=tokens[token_cnt];
            token_cnt+=1;
            retnode.subtree.push_back(leaf2);
        }else{
            syntax_error("symbol ')' ",tokens[token_cnt]);
            exit(0);
        }
    }else if(match(tokens[token_cnt],"number")||match(tokens[token_cnt],"identifier")){
        Treenode leaf1;
        leaf1.token=tokens[token_cnt];
        token_cnt+=1;
        retnode.subtree.push_back(leaf1);
    }else{
        syntax_error("symbol '(' or a number or a identifier",tokens[token_cnt]);
        exit(0);
    }
    parsing_mes_out("parse_factor");
    return retnode;
}

Treenode parse_bool_exp(){//规则20
	parsing_mes_in("parse_bool_exp");
    Treenode retnode;
    retnode.symbol=bool_exp;
    retnode.line=tokens_line[token_cnt];

    while(1){
        Treenode t1;
        t1=parse_bterm();
        retnode.subtree.push_back(t1);
        if(match(tokens[token_cnt],"or")){//
            Treenode leaf;
            leaf.token=tokens[token_cnt];
            token_cnt++;
            retnode.subtree.push_back(leaf);
        }else{//没有+-那就是正常完了，可以回去了
        	parsing_mes_out("parse_bool_exp");
            return retnode;
        }
    }
}

Treenode parse_bterm(){//规则21
	parsing_mes_in("parse_bterm");
    Treenode retnode;
    retnode.symbol=bterm;
    retnode.line=tokens_line[token_cnt];
    while(1){
        Treenode t1;
        t1=parse_bfactor();
        retnode.subtree.push_back(t1);
        if(match(tokens[token_cnt],"and")){//
            Treenode leaf;
            leaf.token=tokens[token_cnt];
            token_cnt++;
            retnode.subtree.push_back(leaf);
        }else{//正常完了，可以回去了
        	parsing_mes_out("parse_bterm");
            return retnode;
        }
    }
}
Treenode parse_bfactor(){//规则22这个直接替换掉了
    return parse_com_exp();
}

Treenode parse_com_exp(){//规则23
	parsing_mes_in("parse_com_exp");
    Treenode retnode;
    retnode.symbol=com_exp;
    retnode.line=tokens_line[token_cnt];

    Treenode t1=parse_arith_exp();
    retnode.subtree.push_back(t1);
    
    //规则24这里直接替换掉了
    string comops[]={"<","=",">","<=",">="};
    int mark=0;
    for(int i=0;i<5;i++){
        if(match(tokens[token_cnt],comops[i])){
            mark=1;
            Treenode leaf;
            leaf.token=tokens[token_cnt];
            token_cnt++;
            retnode.subtree.push_back(leaf);
            break;
        }
    }

    Treenode t2=parse_arith_exp();
    retnode.subtree.push_back(t2);
	parsing_mes_out("parse_com_exp");
    return retnode;
}
// enum syntax_symbols{program,declar,decl,type,varlist,
// stmt_seq,stmt,while_stmt,if_stmt,repeat_stmt,assign_stmt,read_stmt,write_stmt,
// exp,arith_exp,addop,term,mulop,factor,
// bool_exp,bterm,bfactor,com_exp,com_op,str_exp}


//int main(){
//	char fname[100];
//	scanf("%s",fname);
//    FILE* source = fopen(fname, "r");
//    prepare(source);
//    printf("token list:\n");
//    for (int i = 0; i < tokens.size(); i++) {
//        tokens[i].print();
//        cout <<"  ";
//        if(i%5==4)cout<<endl;
//    }
//    cout<<endl<<endl;
//    
//    Treenode root=parse_program();
//    
//    cout<<endl<<"parsing result:"<<endl;
//    root.print(0);
//    return 0;
//}

