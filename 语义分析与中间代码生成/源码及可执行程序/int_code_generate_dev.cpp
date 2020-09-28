#include "syntax_dev.cpp"
#include <map>
//��������
enum var_type{t_int,t_char,t_bool};
//���ű���¼��ʶ����������
map<string,var_type> sym_table;
map<string,int> sym_line;
int gen_dep=0;
//�����Եķ������ڵ�
// struct Treenode{
//     syntax_symbols symbol;
//     vector<Treenode> subtree;
//     Token token;//�������Ҷ�ӽڵ�ʱ����token��
//     void print(int dep);
//     int line;//�ڵ���������
// };
struct nTreenode{
    syntax_symbols symbol;
    vector<nTreenode> subtree;
    Token token;//�������Ҷ�ӽڵ�ʱ����token��
    void print_code();
    void print_tree(int dep);
    int line;//�ڵ���������

    string var;
    var_type type_of_var;//type�Ѿ���ǰ��ռ����
    int begin;
    int next;
    int t_true;
    int t_false;
    vector<string> code;
    void gen_code();
};
void nTreenode::print_code(){
    for(int i=0;i<code.size();i++){
        cout<<code[i]<<endl;
    }
}
void nTreenode::print_tree(int dep){
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
            subtree[i].print_tree(dep+4);
        }
    }
}


//����ͨ�﷨������ת���ɴ����Եķ�����
nTreenode transfrom(Treenode& src){
    nTreenode retnode;
    retnode.symbol=src.symbol;
    retnode.token=src.token;
    retnode.line=src.line;
    for(int i=0;i<src.subtree.size();i++){
        retnode.subtree.push_back(transfrom(src.subtree[i]));
    }
    return retnode;
}

//������ʶ��
void declare(string var_name,var_type type,int line){
    if(sym_table.count(var_name)>0){//�ظ��������
        cout<<"Error at line "<<line<<" :"<<var_name<<" is redefined."<<endl;
        cout<<"It was previously declared in line "<<sym_line[var_name]<<endl;
        exit(0);
    }else{
        sym_line[var_name]=line;
        sym_table[var_name]=type;
     //   cout<<"declare:"<<var_name<<" "<<type;
    }
}

//����ʶ��
void check(string var_name,var_type type,int line){
    if(sym_table.count(var_name)==0){//δ�������
        cout<<"Error at line "<<line<<" :"<<var_name<<" is not defined."<<endl;
        exit(0);
    }else if(sym_table[var_name]!=type){//���ʹ���
        string tstr[]={"'int'","'char'","'bool'"};
        cout<<"Error at line "<<line<<" :"<<" expect type "<<tstr[type];
        cout<<",but "<<var_name<<"'s type is "<<tstr[sym_table[var_name]];
        exit(0);
    }
}

//��ȡ�µ�label
int newlabel(){
    static int label_cnt=0;
    
    cout<<"get a label "<<label_cnt<<endl;
    return label_cnt++;
}
//��ȡ�µ���ʱ����
string newTvar(var_type type,int line){
    static int label_cnt=0;
    char buf[10];
    do{
    	label_cnt++;
    	sprintf(buf,"t%d",label_cnt);
	}while(sym_table.count(buf)>0);
	
    declare(buf,type,line);
    cout<<"get a temp variable t"<<label_cnt<<endl;
    return string(buf);
}

//�ϲ������
void joint(vector<string>& to,vector<string>& b){
	for(int i=0;i<b.size();i++){
		to.push_back(b[i]);
	}
    b.clear();//�Ժ��ò����ˣ�����Լ��ٴ�������
}
//�����ת����label����
string label_str(int label){
    char buf[20];
    sprintf(buf,"Label L%d",label);
    cout<<"placing "<<buf<<endl;
    return buf;
}
//�����ת����goto ��� 
string goto_str(int label){
    char buf[20];
    sprintf(buf,"goto L%d",label);
    cout<<"placing "<<buf<<endl;
    return buf;
}

//�����Ǹ����﷨���������ɴ���

void mes_gen_in(string mes){
	for(int i=0;i<gen_dep;i++){
		if(i%4==3)printf("|");
		else printf(" ");
	}
	cout<<"gen_in "<<mes<<endl;
	gen_dep+=4;
}
void mes_gen_out(string mes){
	gen_dep-=4;
	for(int i=0;i<gen_dep;i++){
		if(i%4==3)printf("|");
		else printf(" ");
	}
	cout<<"gen_out "<<mes<<endl;	
}
void nTreenode::gen_code(){
	
	nTreenode son;
	if(symbol==stmt){
		son=subtree[0];
		mes_gen_in(syntax_symbols_strs[son.symbol]);
	}else{
		mes_gen_in(syntax_symbols_strs[symbol]);
	}
    switch (symbol)
    {
    case program:
        next=newlabel();
        for(int i=0;i<subtree.size();i++){
            if(subtree[i].symbol==stmt_seq){//�̳�����
                subtree[i].next=next;
            }
            subtree[i].gen_code();
            if(subtree[i].symbol==stmt_seq){//�̳�����
                code=subtree[i].code;
                code.push_back(label_str(next));
            }
        }
        break;
    //���￪ʼ����������
    case declar:
        for(int i=0;i<subtree.size();i++){//ÿһ�����������Ըɻ�
            if(subtree[i].subtree.size()!=0)subtree[i].gen_code();
        }
        break;
    
    case decl://decl->type-specifier varlist    //����d1
        subtree[0].gen_code();//type
        subtree[1].type_of_var=subtree[0].type_of_var;//�̳�����
        subtree[1].gen_code();//varlist
        break;

    case type://����d2
        if(subtree[0].token.value=="int"){
            this->type_of_var=t_int;
        }else if(subtree[0].token.value=="bool"){
            this->type_of_var=t_bool;
        }else if(subtree[0].token.value=="char"){
            type_of_var=t_char;
        }else{//�������ᵽ���
            cout<<"program bug! at type"<<endl;
        }
        break;

    case varlist://����d3
        for(int i=0;i<subtree.size();i++){//û�еݹ�����ֱ��ѭ������
            if(subtree[i].token.type==ID){
                declare(subtree[i].token.value,type_of_var,line);
            }
        }
		break;
		
    case stmt_seq://��������stmt //����s1
    	if(subtree.size()==1){
    		subtree[0].next=next;
    		subtree[0].gen_code();
    		code=subtree[0].code;
    	//	code.push_back(label_str(next));
		}else{
	        for(int i=0;i<subtree.size()-1;i++){
	        	if(subtree[i].subtree.size()==0)continue;
	            subtree[i].next=newlabel();
	            subtree[i].gen_code();
	            joint(this->code,subtree[i].code);
	            code.push_back(label_str(subtree[i].next));
	        }
	        subtree[subtree.size()-1].next=this->next;
	        subtree[subtree.size()-1].gen_code();
	        joint(code,subtree[subtree.size()-1].code);
		//	code.push_back(label_str(subtree[subtree.size()-1].next));
		}

        break;

    case stmt:
        
        if(subtree.size()!=1){//��������Ӧ��ֻ��1��
            cout<<"program bug! at stmt,subtreee number error"<<endl;
            exit(0);
        }   

        subtree=subtree[0].subtree;//ֱ�������м�Ľڵ��� 

        //����s2
        if(son.symbol==repeat_stmt){//S->repeat Seq until E
            if(subtree.size()!=4){
                cout<<"program bug! at repeat_stmt"<<endl;
                exit(0);
            }
            begin=newlabel();//S.begin=newlabel;
            subtree[1].next=begin;//Seq.next=S.begin;
            subtree[3].t_true=this->next;//E.true=S.next;
            subtree[3].t_false=this->begin;//E.false=S.begin;
            subtree[1].gen_code();
            subtree[3].gen_code();
            //S.code=Label S.begin||Seq.code||E.code;
            code.push_back(label_str(begin));
            joint(code,subtree[1].code);
            joint(code,subtree[3].code);
        }else if(son.symbol==while_stmt){//����s3
            if(subtree.size()!=5){
                cout<<"program bug! at while_stmt"<<endl;
                exit(0);
            }
            begin=newlabel();
            subtree[3].next=begin;
            subtree[1].t_true=newlabel();
            subtree[1].t_false=next;

            subtree[1].gen_code();
            subtree[3].gen_code();
            code.push_back(label_str(begin));
            joint(code,subtree[1].code);
            code.push_back(label_str(subtree[1].t_true));
            joint(code,subtree[3].code);
            code.push_back(goto_str(begin));
        }else if(son.symbol==if_stmt){
            if(subtree.size()==5){//����s4
                subtree[1].t_true=newlabel();
                subtree[1].t_false=next;
                subtree[3].next=next;
                
                subtree[1].gen_code();
                subtree[3].gen_code();
                joint(code,subtree[1].code);
                code.push_back(label_str(subtree[1].t_true));
                joint(code,subtree[3].code);
            }else if(subtree.size()==7){//����s5
                subtree[1].t_true=newlabel();
                subtree[1].t_false=newlabel();
                subtree[3].next=next;
                subtree[5].next=next;

                subtree[1].gen_code();
                subtree[3].gen_code();
                subtree[5].gen_code();
                joint(code,subtree[1].code);
                code.push_back(label_str(subtree[1].t_true));
                joint(code,subtree[3].code);
                code.push_back(goto_str(next));
                code.push_back(label_str(subtree[1].t_false));
                joint(code,subtree[5].code);

            }else{
                cout<<"program bug! at if_stmt"<<endl;
                exit(0);
            }
        }else if(son.symbol==assign_stmt){//����s6
            if(subtree.size()!=3){
                cout<<"program bug! at assign_stmt"<<endl;
                exit(0);
            }
            subtree[2].gen_code();
            check(subtree[0].token.value,subtree[2].type_of_var,line);
            joint(code,subtree[2].code);
            code.push_back(subtree[0].token.value+":="+subtree[2].var);
        }else if(son.symbol==read_stmt){////����s7
            if(subtree.size()!=2){
                cout<<"program bug! at read_stmt"<<endl;
                exit(0);
            }
            code.push_back(string("read ")+subtree[1].token.value);
        }else if(son.symbol==write_stmt){//����s8
            if(subtree.size()!=2){
                cout<<"program bug! at write_stmt"<<endl;
                exit(0);
            }
            subtree[1].gen_code();
            joint(code,subtree[1].code);
            code.push_back(string("write ")+subtree[1].var);
        }else{
            cout<<"program bug! at stmt, subtree type error"<<endl;
            exit(0);   
        }
        break;

    case exp://����e1
        if(subtree.size()!=1){//��������Ӧ��ֻ��1��
            cout<<"program bug! at exp,subtreee number error"<<endl;
            exit(0);
        }   
        if(subtree[0].symbol==arith_exp){
            subtree[0].gen_code();
            code=subtree[0].code;
            var=subtree[0].var;
            type_of_var=t_int;
        }else if(subtree[0].symbol==bool_exp){
            subtree[0].t_true=t_true;
            subtree[0].t_false=t_false;

            subtree[0].gen_code();
            code=subtree[0].code;
            var=subtree[0].var;
            type_of_var=t_bool;
        }else if(subtree[0].token.type==STR){
            var="\""+subtree[0].token.value+"\"";
            type_of_var=t_char;
        }
        break;

    case arith_exp://����e2
        if(subtree.size()%2!=1){
            cout<<"program bug! at arith_exp,subtreee number error"<<endl;
            exit(0);
        }
        if(subtree.size()==1){
            subtree[0].gen_code();
            var=subtree[0].var;
            code=subtree[0].code;
        }else{
            var=newTvar(t_int,line);
            type_of_var=t_int;
            subtree[0].gen_code();
            code=subtree[0].code;
            for(int i=1;2*i<subtree.size();i++){
                subtree[2*i].gen_code();
                joint(code,subtree[2*i].code);
                code.push_back(var+":="+subtree[2*i-2].var+subtree[2*i-1].token.value+subtree[2*i].var);
            }
        }
        break;

    case term://����e3
        if(subtree.size()%2!=1){
            cout<<"program bug! at term,subtreee number error"<<endl;
            exit(0);
        }
        if(subtree.size()==1){
            subtree[0].gen_code();
            var=subtree[0].var;
            code=subtree[0].code;
        }else{
            var=newTvar(t_int,line);
            type_of_var=t_int;
            subtree[0].gen_code();
            code=subtree[0].code;
            for(int i=1;2*i<subtree.size();i++){
                subtree[2*i].gen_code();
                joint(code,subtree[2*i].code);
                code.push_back(var+":="+subtree[2*i-2].var+subtree[2*i-1].token.value+subtree[2*i].var);
            }
        }
        break;

    case factor://����e4
        if(subtree.size()==3){
            subtree[1].gen_code();
            var=subtree[1].var;
            code=subtree[1].code;
        }else if(subtree.size()==1){
            if(subtree[0].token.type==NUM){
                var=subtree[0].token.value;
            }else if(subtree[0].token.type==ID){
                var=subtree[0].token.value;
                check(var,t_int,line);
            }else{
                cout<<"program bug! at factor,token type error"<<endl;
                exit(0);
            }
        }else{
            cout<<"program bug! at factor,subtreee number error"<<endl;
            exit(0);
        }
        break;    

    case bool_exp://����e5
        if(subtree.size()%2!=1){
            cout<<"program bug! at bool_exp,subtreee number error"<<endl;
            exit(0);
        }
        if(subtree.size()==1){

            subtree[0].t_true=t_true;
            subtree[0].t_false=t_false;
            subtree[0].gen_code();
            var=subtree[0].var;
            code=subtree[0].code;
        }else{
            var=newTvar(t_bool,line);
            type_of_var=t_bool;
            subtree[0].t_true=t_true;
        //    subtree[0].t_false=t_false;
            subtree[0].t_false=newlabel();
            subtree[0].gen_code();
            code=subtree[0].code;
            for(int i=1;2*i<subtree.size();i++){
                code.push_back(label_str(subtree[2*i-2].t_false));
                subtree[i].t_true=t_true;
                subtree[i].t_false=newlabel();
                subtree[2*i].gen_code();
                joint(code,subtree[2*i].code);
            }
        }
        break;    

    case bterm://����e6
        if(subtree.size()%2!=1){
            cout<<"program bug! at bterm,subtreee number error"<<endl;
            exit(0);
        }
        if(subtree.size()==1){
            subtree[0].t_true=t_true;
            subtree[0].t_false=t_false;
            subtree[0].gen_code();
            var=subtree[0].var;
            code=subtree[0].code;
        }else{
            var=newTvar(t_bool,line);
            type_of_var=t_bool;
            subtree[0].t_true=newlabel();
           	//subtree[0].t_true=t_true;
            subtree[0].t_false=t_false;
            subtree[0].gen_code();
            code=subtree[0].code;
            int i=1;
            for(;2*(i+1)<subtree.size();i++){
                code.push_back(label_str(subtree[2*i-2].t_true));
                subtree[2*i].t_true=newlabel();
            	subtree[2*i].t_false=t_false;
                subtree[2*i].gen_code();
                joint(code,subtree[2*i].code);
            }
            code.push_back(label_str(subtree[2*i-2].t_true));
            subtree[2*i].t_true=t_true;
        	subtree[2*i].t_false=t_false;
            subtree[2*i].gen_code();
            joint(code,subtree[2*i].code); 
        }
        break; 

    case com_exp://����e7
        if(subtree.size()!=3){
            cout<<"program bug! at com_exp,subtreee number error"<<endl;
            exit(0);
        }
        var=newTvar(t_bool,line);
        type_of_var=t_bool;
        subtree[0].gen_code();
        subtree[2].gen_code();
        joint(code,subtree[0].code);
        joint(code,subtree[2].code);
        code.push_back("if "+subtree[0].var+subtree[1].token.value+subtree[2].var+" "+goto_str(t_true));
        code.push_back(goto_str(t_false));
        break;
        
    default:
        cout<<"program bug! error type to gen_code():"<<symbol<<endl;
        exit(0);
        break;
    }
    mes_gen_out(syntax_symbols_strs[symbol]);
    return;
}

int main(){
	char fname[100];
	scanf("%s",fname);
    FILE* source = fopen(fname, "r");
    //�ʷ����� 
    prepare(source);
//    printf("token list:\n");
//    for (int i = 0; i < tokens.size(); i++) {
//        tokens[i].print();
//        cout <<"  ";
//        if(i%5==4)cout<<endl;
//    }
//    cout<<endl<<endl;
    //�﷨���� 
    Treenode root=parse_program();
//    cout<<endl<<"parsing result:"<<endl;
//    root.print(0);
	//������ת�� 
    nTreenode n_root=transfrom(root);
//    n_root.print_tree(0);
	//�����м���� 
    n_root.gen_code();
    cout<<endl<<endl<<"int_code:"<<endl;
    //����м���� 
    n_root.print_code();
    return 0;
}
