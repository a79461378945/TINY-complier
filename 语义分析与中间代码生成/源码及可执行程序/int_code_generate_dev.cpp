#include "syntax_dev.cpp"
#include <map>
//变量类型
enum var_type{t_int,t_char,t_bool};
//符号表，记录标识符及其类型
map<string,var_type> sym_table;
map<string,int> sym_line;
int gen_dep=0;
//带属性的分析树节点
// struct Treenode{
//     syntax_symbols symbol;
//     vector<Treenode> subtree;
//     Token token;//当这个是叶子节点时就是token了
//     void print(int dep);
//     int line;//节点所在行数
// };
struct nTreenode{
    syntax_symbols symbol;
    vector<nTreenode> subtree;
    Token token;//当这个是叶子节点时就是token了
    void print_code();
    void print_tree(int dep);
    int line;//节点所在行数

    string var;
    var_type type_of_var;//type已经被前面占用了
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
    if(subtree.size()==0){//叶子节点
        token.print();
        printf("\n");
    }else{
        cout<<syntax_symbols_strs[symbol]<<endl;
        for(int i=0;i<subtree.size();i++){
            subtree[i].print_tree(dep+4);
        }
    }
}


//将普通语法分析树转换成带属性的分析树
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

//声明标识符
void declare(string var_name,var_type type,int line){
    if(sym_table.count(var_name)>0){//重复定义错误
        cout<<"Error at line "<<line<<" :"<<var_name<<" is redefined."<<endl;
        cout<<"It was previously declared in line "<<sym_line[var_name]<<endl;
        exit(0);
    }else{
        sym_line[var_name]=line;
        sym_table[var_name]=type;
     //   cout<<"declare:"<<var_name<<" "<<type;
    }
}

//检查标识符
void check(string var_name,var_type type,int line){
    if(sym_table.count(var_name)==0){//未定义错误
        cout<<"Error at line "<<line<<" :"<<var_name<<" is not defined."<<endl;
        exit(0);
    }else if(sym_table[var_name]!=type){//类型错误
        string tstr[]={"'int'","'char'","'bool'"};
        cout<<"Error at line "<<line<<" :"<<" expect type "<<tstr[type];
        cout<<",but "<<var_name<<"'s type is "<<tstr[sym_table[var_name]];
        exit(0);
    }
}

//获取新的label
int newlabel(){
    static int label_cnt=0;
    
    cout<<"get a label "<<label_cnt<<endl;
    return label_cnt++;
}
//获取新的临时变量
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

//合并代码的
void joint(vector<string>& to,vector<string>& b){
	for(int i=0;i<b.size();i++){
		to.push_back(b[i]);
	}
    b.clear();//以后都用不上了，清空以减少储存消耗
}
//将序号转换成label代码
string label_str(int label){
    char buf[20];
    sprintf(buf,"Label L%d",label);
    cout<<"placing "<<buf<<endl;
    return buf;
}
//将序号转换成goto 语句 
string goto_str(int label){
    char buf[20];
    sprintf(buf,"goto L%d",label);
    cout<<"placing "<<buf<<endl;
    return buf;
}

//下面是根据语法分析树生成代码

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
            if(subtree[i].symbol==stmt_seq){//继承属性
                subtree[i].next=next;
            }
            subtree[i].gen_code();
            if(subtree[i].symbol==stmt_seq){//继承属性
                code=subtree[i].code;
                code.push_back(label_str(next));
            }
        }
        break;
    //这里开始是声明部分
    case declar:
        for(int i=0;i<subtree.size();i++){//每一个声明语句各自干活
            if(subtree[i].subtree.size()!=0)subtree[i].gen_code();
        }
        break;
    
    case decl://decl->type-specifier varlist    //规则d1
        subtree[0].gen_code();//type
        subtree[1].type_of_var=subtree[0].type_of_var;//继承属性
        subtree[1].gen_code();//varlist
        break;

    case type://规则d2
        if(subtree[0].token.value=="int"){
            this->type_of_var=t_int;
        }else if(subtree[0].token.value=="bool"){
            this->type_of_var=t_bool;
        }else if(subtree[0].token.value=="char"){
            type_of_var=t_char;
        }else{//正常不会到这的
            cout<<"program bug! at type"<<endl;
        }
        break;

    case varlist://规则d3
        for(int i=0;i<subtree.size();i++){//没有递归所以直接循环即可
            if(subtree[i].token.type==ID){
                declare(subtree[i].token.value,type_of_var,line);
            }
        }
		break;
		
    case stmt_seq://子树都是stmt //规则s1
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
        
        if(subtree.size()!=1){//正常子树应该只有1个
            cout<<"program bug! at stmt,subtreee number error"<<endl;
            exit(0);
        }   

        subtree=subtree[0].subtree;//直接跳过中间的节点了 

        //规则s2
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
        }else if(son.symbol==while_stmt){//规则s3
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
            if(subtree.size()==5){//规则s4
                subtree[1].t_true=newlabel();
                subtree[1].t_false=next;
                subtree[3].next=next;
                
                subtree[1].gen_code();
                subtree[3].gen_code();
                joint(code,subtree[1].code);
                code.push_back(label_str(subtree[1].t_true));
                joint(code,subtree[3].code);
            }else if(subtree.size()==7){//规则s5
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
        }else if(son.symbol==assign_stmt){//规则s6
            if(subtree.size()!=3){
                cout<<"program bug! at assign_stmt"<<endl;
                exit(0);
            }
            subtree[2].gen_code();
            check(subtree[0].token.value,subtree[2].type_of_var,line);
            joint(code,subtree[2].code);
            code.push_back(subtree[0].token.value+":="+subtree[2].var);
        }else if(son.symbol==read_stmt){////规则s7
            if(subtree.size()!=2){
                cout<<"program bug! at read_stmt"<<endl;
                exit(0);
            }
            code.push_back(string("read ")+subtree[1].token.value);
        }else if(son.symbol==write_stmt){//规则s8
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

    case exp://规则e1
        if(subtree.size()!=1){//正常子树应该只有1个
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

    case arith_exp://规则e2
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

    case term://规则e3
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

    case factor://规则e4
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

    case bool_exp://规则e5
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

    case bterm://规则e6
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

    case com_exp://规则e7
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
    //词法分析 
    prepare(source);
//    printf("token list:\n");
//    for (int i = 0; i < tokens.size(); i++) {
//        tokens[i].print();
//        cout <<"  ";
//        if(i%5==4)cout<<endl;
//    }
//    cout<<endl<<endl;
    //语法分析 
    Treenode root=parse_program();
//    cout<<endl<<"parsing result:"<<endl;
//    root.print(0);
	//分析树转换 
    nTreenode n_root=transfrom(root);
//    n_root.print_tree(0);
	//生成中间代码 
    n_root.gen_code();
    cout<<endl<<endl<<"int_code:"<<endl;
    //输出中间代码 
    n_root.print_code();
    return 0;
}
