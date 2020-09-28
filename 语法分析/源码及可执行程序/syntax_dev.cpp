#include "lexical_dev.cpp"
//���ȶ����ķ����� ���ս��
enum syntax_symbols{program,declar,decl,type,varlist,
stmt_seq,stmt,while_stmt,if_stmt,repeat_stmt,assign_stmt,read_stmt,write_stmt,
exp,arith_exp,addop,term,mulop,factor,
bool_exp,bterm,bfactor,com_exp,com_op,str_exp};
string syntax_symbols_strs[]={"program","declar","decl","type","varlist",
"stmt_seq","stmt","while_stmt","if_stmt","repeat_stmt","assign_stmt","read_stmt","write_stmt",
"exp","arith_exp","addop","term","mulop","factor",
"bool_exp","bterm","bfactor","com_exp","com_op","str_exp"};
//���addop,bfactor,com_opû��
int token_cnt=0;//���µ�ǰ������token��
vector<int> tokens_line;//��һ�����token��Ӧ����
int parsing_depth;

//�����﷨���ڵ�
struct Treenode{
    syntax_symbols symbol;
    vector<Treenode> subtree;
    Token token;//�������Ҷ�ӽڵ�ʱ����token��
    void print(int dep);
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
    if(subtree.size()==0){//Ҷ�ӽڵ�
        token.print();
        printf("\n");
    }else{
        cout<<syntax_symbols_strs[symbol]<<endl;
        for(int i=0;i<subtree.size();i++){
            subtree[i].print(dep+4);
        }
    }
}

//��ֱ�ӵõ�����token������˵
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

void parsing_mes_in(string mes){//�����ڷ����������������֪���������� 
	for(int i=0;i<parsing_depth;i++){
		if(i%4==3)printf("|");
		else printf(" ");
	}
	cout<<mes<<" in ";
	tokens[token_cnt].print();
	cout<<endl;
	parsing_depth+=4;
}
void parsing_mes_out(string mes){//�����ڷ����������������֪���������� 
	parsing_depth-=4;
	for(int i=0;i<parsing_depth;i++){
		if(i%4==3)printf("|");
		else printf(" ");
	}
	cout<<mes<<" out ";
	tokens[token_cnt].print();
	cout<<endl;
}
//�ս������token��ƥ��
//�ս������ ������ţ��ؼ��֣�ID��string ��NUM
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
    for(int i=0;i<15;i++){//���Ƿ�Ҫƥ��ؼ���
        if(t_sym==keywords[i]){
            if(t.type==KEY && t.value==t_sym){
                return true;
            }else{
                return false;
            }
        }
    }
    for(int i=0;i<15;i++){//���Ƿ�Ҫƥ�����
        if(t_sym==symbols[i]){
            if(t.type==SYM && t.value==t_sym){
                return true;
            }else{
                return false;
            }
        }
    }
    //�������򲻹�����Դ����ζ�Ӧ���ǲ��ᵽ���
    cout<<"program bug:there is no "<<t_sym<<endl;
    exit(0);
}

//Ȼ����Կ�ʼ��

//first declarations����Ϊ�ա�int��bool��char
//first stmt-sequence����Ϊwhile,if,repeat,identifier,read,write

Treenode parse_program(){//���� ��Ӧ����1
    parsing_mes_in("parse_program");
    Treenode retnode;
    retnode.symbol=program;
    
    Treenode declarations=parse_declar();
    retnode.subtree.push_back(declarations);
    
    Treenode stmt_sequence=parse_stmt_seq();
    retnode.subtree.push_back(stmt_sequence);
    
    parsing_mes_out("parse_program");
    return retnode;
}
Treenode parse_declar(){//�����������  ��Ӧ����2
	parsing_mes_in("parse_declar");
    Treenode retnode;
    retnode.symbol=declar;
    while(match(tokens[token_cnt],"int")||match(tokens[token_cnt],"bool")||match(tokens[token_cnt],"char")){
        retnode.subtree.push_back(parse_decl());
        if(!match(tokens[token_cnt],";")){//����������˾�Ȼû��; �﷨��
            syntax_error(";",tokens[token_cnt]);
        }else{//���Ǽ���Ȼ�����һ��
            Treenode leaf;
            leaf.token=tokens[token_cnt];
            token_cnt++;
            retnode.subtree.push_back(leaf); 
        }
    }
    parsing_mes_out("parse_declar");
    return retnode;
}

Treenode parse_stmt_seq(){//������� ��Ӧ����6
	parsing_mes_in("parse_stmt_seq");
    Treenode retnode;
    retnode.symbol=stmt_seq;

    int mark=0;//���ٵ�ƥ��һ��
    while(1){
        Treenode t1;
        t1=parse_stmt();
        retnode.subtree.push_back(t1);
        if(match(tokens[token_cnt],";")){//��;˵�����е�ƥ��
            Treenode leaf;
            leaf.token=tokens[token_cnt];
            token_cnt++;
            retnode.subtree.push_back(leaf);
        }else{//û�ж������Ǿ����������ˣ����Ի�ȥ��
        	parsing_mes_out("parse_stmt_seq");
            return retnode;
        }
    }

}

Treenode parse_decl(){//����������� ��Ӧ����3
    //֮ǰparse_declar�Ѿ�ȷ����ͷ��specifier��
    parsing_mes_in("parse_decl");
    Treenode retnode;
    retnode.symbol=decl;
    Treenode s1=parse_type();
    retnode.subtree.push_back(s1);
    Treenode s2=parse_varlist();
    retnode.subtree.push_back(s2);
    parsing_mes_out("parse_decl");
    return retnode;
}
Treenode parse_type(){//���ͱ�ʶ�� ��Ӧ����4
	parsing_mes_in("parse_type");
    Treenode retnode;
    retnode.symbol=type;
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
    //ûƥ���ϣ�������Ҳ���ᵽ���
    syntax_error("type-specifier",tokens[token_cnt]);
    exit(0);
}
Treenode parse_varlist(){//������ʶ���б� ��Ӧ����5
	parsing_mes_in("parse_varlist");
    Treenode retnode;
    retnode.symbol=varlist;
    int mark=0;//���ٵ�ƥ��һ��
    while(match(tokens[token_cnt],"identifier")){
        mark=1;
        Treenode leaf1;
        leaf1.token=tokens[token_cnt];
        token_cnt++;
        retnode.subtree.push_back(leaf1);
        if(match(tokens[token_cnt],",")){//�ж���˵�����е�ƥ��
            Treenode leaf2;
            leaf2.token=tokens[token_cnt];
            token_cnt++;
            retnode.subtree.push_back(leaf2);
        }else{//û�ж������Ǿ����������ˣ����Ի�ȥ��
        	parsing_mes_out("parse_varlist");
            return retnode;
        }
    }
    // û�н��ϱ�ʶ�����﷨������
    syntax_error("identifier",tokens[token_cnt]);
    exit(0);
}

Treenode parse_stmt(){// ��Ӧ����7 
	parsing_mes_in("parse_stmt");
    Treenode retnode;
    retnode.symbol=stmt;
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

Treenode parse_while_stmt(){//����8
	parsing_mes_in("parse_while_stmt");
    Treenode retnode;
    retnode.symbol=while_stmt;

    Treenode leaf1;//����֮ǰ�Ѿ�֪����ǰtoken��while��
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
Treenode parse_if_stmt(){//����9
	parsing_mes_in("parse_if_stmt");
    Treenode retnode;
    retnode.symbol=if_stmt;

    Treenode leaf1;//����֮ǰ�Ѿ�֪����ǰtoken��if��
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

    if(match(tokens[token_cnt],"else")){//����else �Ļ�
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
Treenode parse_repeat_stmt(){//����10
	parsing_mes_in("parse_repeat_stmt");
    Treenode retnode;
    retnode.symbol=repeat_stmt;

    Treenode leaf1;//����֮ǰ�Ѿ�֪����ǰtoken��repeat��
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
Treenode parse_assign_stmt(){//����11
	parsing_mes_in("parse_assign_stmt");
    Treenode retnode;
    retnode.symbol=assign_stmt;

    Treenode leaf1;//����֮ǰ�Ѿ�֪����ǰtoken��ID��
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
Treenode parse_read_stmt(){//����12
	parsing_mes_in("parse_read_stmt");
    Treenode retnode;
    retnode.symbol=read_stmt;

    Treenode leaf1;//����֮ǰ�Ѿ�֪����ǰtoken��read��
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
Treenode parse_write_stmt(){//����13
	parsing_mes_in("parse_write_stmt");
    Treenode retnode;
    retnode.symbol=write_stmt;

    Treenode leaf1;//����֮ǰ�Ѿ�֪����ǰtoken��write��
    leaf1.token=tokens[token_cnt];
    token_cnt+=1;
    retnode.subtree.push_back(leaf1);

    Treenode t1;
    t1=parse_exp();
    retnode.subtree.push_back(t1);
	parsing_mes_out("parse_write_stmt");
    return retnode;
}


Treenode parse_exp(){//����14
	parsing_mes_in("parse_exp");
    Treenode retnode;
    retnode.symbol=exp;
    Treenode t1;
    if(match(tokens[token_cnt],"string")){//��str�Ǿ�ֱ�Ӻ��ж�
        t1.token=tokens[token_cnt];
        token_cnt += 1;
    }else{//����bool-expҲ������arithmetic-exp��ͷ���������ﻹ�����ж�
        //Ҳ����ͨ�������ķ�����ķ�ʽ�������������ֱ�Ӽ���������
        //���ҵ�follow(exp)=follow(write-stmt)+follow(assign-stmt)=follow(statement)
        // follow(statement)= ';' + follow(stmt-sequence) 
        //  = ';' + '$' + "end" + "else" + "until"
        string follow_exp[]={";","$","end","else","until"};
        string comops[]={"<","=",">","<=",">="};
        int i=1;//�ӵ�ǰ֮���һ����ʼ�ң���Ϊarithmetic-exp��first��Ϊ��
        int bool_mark=0;

        while(1){
            int end_mark=0;
            for(int j=0;j<5;j++){//�Ƿ��Ѿ�����exo֮����
                if(match(tokens[token_cnt+i],follow_exp[j])){
                    end_mark=1;
                }
            }
            if(end_mark==1)break;
            for(int j=0;j<5;j++){//������û�бȽ������
                if(match(tokens[token_cnt+i],comops[j])){
                    bool_mark=1;
                    end_mark=1;
                    break;
                }
            }
            if(end_mark==1)break;
            i++;
            if(i>=tokens.size()){//�����������һ��token�ǽ�����Ӧ�ò���ִ�е������
                cout<<"program bug,exceed tokens' size"<<endl;
                exit(0);
            }
        }
        //�����Ѿ�֪����ѡ���ĸ����ʽ��
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

Treenode parse_arith_exp(){//����15
	parsing_mes_in("parse_arith_exp");
    Treenode retnode;
    retnode.symbol=arith_exp;

    while(1){
        Treenode t1;
        t1=parse_term();
        retnode.subtree.push_back(t1);
        //����16ֱ���滻�����˲�����д������
        if(match(tokens[token_cnt],"+")||match(tokens[token_cnt],"-")){//��;˵�����е�ƥ��
            Treenode leaf;
            leaf.token=tokens[token_cnt];
            token_cnt++;
            retnode.subtree.push_back(leaf);
        }else{//û��+-�Ǿ����������ˣ����Ի�ȥ��
        	parsing_mes_out("parse_arith_exp");
            return retnode;
        }
    }
}
Treenode parse_term(){//����17
	parsing_mes_in("parse_term");
    Treenode retnode;
    retnode.symbol=term;
    while(1){
        Treenode t1;
        t1=parse_factor();
        retnode.subtree.push_back(t1);
        //����18ֱ���滻�����˲�����д������
        if(match(tokens[token_cnt],"*")||match(tokens[token_cnt],"/")){//��;˵�����е�ƥ��
            Treenode leaf;
            leaf.token=tokens[token_cnt];
            token_cnt++;
            retnode.subtree.push_back(leaf);
        }else{//û��* / �Ǿ����������ˣ����Ի�ȥ��
        	parsing_mes_out("parse_term");
            return retnode;
        }
    }
}
Treenode parse_factor(){//����19
	parsing_mes_in("parse_factor");
    Treenode retnode;
    retnode.symbol=factor;
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

Treenode parse_bool_exp(){//����20
	parsing_mes_in("parse_bool_exp");
    Treenode retnode;
    retnode.symbol=bool_exp;

    while(1){
        Treenode t1;
        t1=parse_bterm();
        retnode.subtree.push_back(t1);
        if(match(tokens[token_cnt],"or")){//
            Treenode leaf;
            leaf.token=tokens[token_cnt];
            token_cnt++;
            retnode.subtree.push_back(leaf);
        }else{//û��+-�Ǿ����������ˣ����Ի�ȥ��
        	parsing_mes_out("parse_bool_exp");
            return retnode;
        }
    }
}

Treenode parse_bterm(){//����21
	parsing_mes_in("parse_bterm");
    Treenode retnode;
    retnode.symbol=bterm;
    while(1){
        Treenode t1;
        t1=parse_bfactor();
        retnode.subtree.push_back(t1);
        if(match(tokens[token_cnt],"and")){//
            Treenode leaf;
            leaf.token=tokens[token_cnt];
            token_cnt++;
            retnode.subtree.push_back(leaf);
        }else{//�������ˣ����Ի�ȥ��
        	parsing_mes_out("parse_bterm");
            return retnode;
        }
    }
}
Treenode parse_bfactor(){//����22���ֱ���滻����
    return parse_com_exp();
}

Treenode parse_com_exp(){//����23
	parsing_mes_in("parse_com_exp");
    Treenode retnode;
    retnode.symbol=com_exp;

    Treenode t1=parse_arith_exp();
    retnode.subtree.push_back(t1);
    
    //����24����ֱ���滻����
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


int main(){
	char fname[100];
	scanf("%s",fname);
    FILE* source = fopen(fname, "r");
    prepare(source);
    printf("token list:\n");
    for (int i = 0; i < tokens.size(); i++) {
        tokens[i].print();
        cout <<"  ";
        if(i%5==4)cout<<endl;
    }
    cout<<endl<<endl;
    
    Treenode root=parse_program();
    
    cout<<endl<<"parsing result:"<<endl;
    root.print(0);
    return 0;
}

