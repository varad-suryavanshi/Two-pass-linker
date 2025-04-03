#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <math.h>
#include <ctype.h>

FILE *file; 
int buffer_size = 4096;  
char *buffer;
int linenum=0;
int lineoffset=0;
char delimiter[]={' ','\t','\n'};
int module_add[10];
int j=0;
char *token;
int previous;
void __parseerror(int errcode) {
    static char* errstr[] = {
        "TOO_MANY_DEF_IN_MODULE", // > 16
        "TOO_MANY_USE_IN_MODULE", // > 16
        "TOO_MANY_INSTR", // total num_instr exceeds memory size (512)
        "NUM_EXPECTED", // Number expect, anything >= 2^30 is not a number either
        "SYM_EXPECTED", // Symbol Expected
        "MARIE_EXPECTED", // Addressing Expected which is M/A/R/I/E
        "SYM_TOO_LONG", // Symbol Name is too long
    };
    int LO;
    if(token != NULL) {
        int x=strlen(token);
        LO=lineoffset-strlen(token)+1;
    }
    else LO=previous+1;
    
    printf("Parse Error line %d offset %d: %s\n", linenum, LO, errstr[errcode]);
    exit(1);
}


void get_new_line() {
    if (fgets(buffer, buffer_size, file) == NULL) {
        
        buffer = NULL;
    }
    else if (buffer[0] == '\n') {
        linenum++;
        if (fgets(buffer, buffer_size, file) == NULL) {
            if (feof(file)) {
                printf("Reached end of file after empty line.\n");
                
            } else {
                perror("Error reading from file after empty line");
            }
        }
        linenum++;
    }
    else {
        linenum++;
    }
    
}

char* gettoken(){
    
    if(buffer == NULL){
        return NULL;
    }
    if(lineoffset == 0){
        token=strtok(buffer,delimiter);
        previous=lineoffset;
        if(token!=NULL) lineoffset+=strlen(token);
    }
    else{
        token= strtok(NULL,delimiter);
        if(token == NULL){
            previous=lineoffset;
            lineoffset=0;
            get_new_line();
            token=gettoken();         
        }
        else{
            previous=lineoffset;
            if(token!=NULL) lineoffset+=strlen(token)+1;
        }
    }
    return token;
}

int readInt(){
    char *tn=gettoken();
    char *tok=tn;
    if (tn == NULL ){
        return -1;
    }
    int res=1;
    while (*tn) {
        if (!isdigit(*tn)) {
            res = 0;  
        }
        tn++;
    }
    
    if (res == 0 || atoi(tok)>= pow(2, 30)) __parseerror(3);
    
    return (atoi(tok));
}

char* readSym(){
    char *tn=gettoken();
    if(tn==NULL )  __parseerror(4);
    if(strlen(tn)>16)  __parseerror(6);
    regex_t regex;
    int reti;

    reti = regcomp(&regex, "^[A-Za-z][A-Za-z0-9]*$", REG_EXTENDED);
    reti = regexec(&regex, tn, 0, NULL, 0);
    regfree(&regex); 

    if (!reti) {
        return tn; 
    } 
    else {
        __parseerror(4);
        return "";
    }
    
}

char readMARIE() {
    char *tn = gettoken();
    if (tn == NULL) {
        __parseerror(5);  
        return '\0';
    }

    if (tn[0] == 'M' || tn[0] == 'A' || tn[0] == 'R' || tn[0] == 'I' || tn[0] == 'E') {
        return tn[0];
    } else {
        __parseerror(5); 
        return '\0';
    }
}
// Define the structure to hold symbol and its address
struct SymbolTableEntry {
    char *symbol;
    int address;
    int mod_n;
};

// Global symbol table and its size
struct SymbolTableEntry *symbolTable;
int symbolCount = 0;
int symbol_flag[256];

void createSymbol(char *s,int v,int a,int mod_no){
    symbolCount++;
    symbolTable = realloc(symbolTable, symbolCount * sizeof(struct SymbolTableEntry));
    if (symbolTable == NULL) {
        perror("Failed to allocate memory for symbol table");
        exit(EXIT_FAILURE);
    }
    if(s != NULL){
        if(strlen(s)>16){
            printf("The length of the Symbol is greater than 16");
            exit(2);
        }
        symbolTable[symbolCount - 1].symbol = malloc((strlen(s) + 1) * sizeof(char)); 
        if (symbolTable[symbolCount - 1].symbol == NULL) {
            perror("Failed to allocate memory for symbol");
            exit(EXIT_FAILURE);
        }
        strcpy(symbolTable[symbolCount - 1].symbol, s);
        symbolTable[symbolCount-1].mod_n=mod_no;
        symbolTable[symbolCount - 1].address = v + a;
    }    
}

int f[100];
int Pass1() {
    int defcount;
    module_add[0]=0;
    while (1) {
        
        defcount = readInt(); 
        if (defcount >16) __parseerror(0);
        if (defcount < 0) return 0; 
        
        for(int i=0;i<defcount;i++) f[i]=0;
        int def_val[defcount];
        for(int i=0;i<defcount;i++) def_val[i]=0;
        char** def_var=malloc(defcount * sizeof(char *));
        for (int i=0;i<defcount;i++) {
            char *tok= readSym();
            if(tok != NULL) {
                def_var[i]= malloc((strlen(tok)+1) * sizeof(char));
                strcpy(def_var[i],tok);
            }
            def_val[i] = readInt();
            
            if (def_var!=NULL){
                for(int j=0;j<symbolCount;j++){
                    if (strcmp(symbolTable[j].symbol, def_var[i]) == 0){
                        f[i]=1;
                        symbol_flag[j]=1;
                    }
                }
                if (f[i] !=1) createSymbol(def_var[i],def_val[i],module_add[j],j);
                
            }
        }
        int usecount = readInt();
        if (usecount >16) __parseerror(1);
        for (int i=0;i<usecount;i++) {
            char* s = readSym();
        }
        int instcount = readInt();
        if ((instcount + module_add[j]) >512) __parseerror(2);
        for(int i=0;i<defcount;i++){
            if (f[i] ==0 && def_val[i]>=instcount){
                
                int num=-1;
                for(int c=0;c<symbolCount;c++){
                    if(strcmp(symbolTable[c].symbol, def_var[i]) == 0) num=c;
                }
                symbolTable[num].address = module_add[j];
            }
        }
        for (int i=0;i<instcount;i++) {
            char addressmode = readMARIE();
            int operand = readInt();     
        }
        for(int i=0;i<defcount;i++) if (f[i] ==1) printf("Warning: Module %d: %s redefinition ignored\n",j,def_var[i]);
        for(int i=0;i<defcount;i++){
            if (f[i] ==0 && def_val[i]>=instcount){
                printf("Warning: Module %d: %s=%d valid=[0..%d] assume zero relative\n",j,def_var[i],def_val[i],(instcount-1));
            }
        }
        j++;
        module_add[j]=module_add[j-1]+instcount;
        for (int i = 0; i < defcount; i++) {
            free(def_var[i]);  
        }
        free(def_var);  
    } 
    return 0;
}
int mem_addr=0;
int *uselist_flag;
int *deflist_flag;

int Pass2(){
    int mod=0;
    deflist_flag = malloc(symbolCount * sizeof(int)); 
    for (int i = 0; i < symbolCount; i++) {
        deflist_flag[i] = 0; 
    }

    while (1) {
        
        int defcount = readInt(); 
        if (defcount < 0) return 0; 
        int val;
        for (int i=0;i<defcount;i++) {
            char* sym = readSym();
            val = readInt();
        }
        int usecount = readInt();
        char **usesymbol;
        usesymbol=malloc(usecount * sizeof(char*));
        uselist_flag = malloc(usecount * sizeof(int)); 
        for (int i = 0; i < usecount; i++) {
            uselist_flag[i] = 0; 
        }

        for (int i=0;i<usecount;i++) {
            char *sym = readSym();
            if (sym != NULL) {
                usesymbol[i] = malloc((strlen(sym) + 1) * sizeof(char));
                strcpy(usesymbol[i], sym);
                uselist_flag[i]=0;
            }
            
        }
        int instcount = readInt();
        for (int i=0;i<instcount;i++) {
            char addressmode = readMARIE();
            int operand = readInt();
            
            if(addressmode == 'A'){
                if (operand/1000 >=10){
                    printf("%03d: %04d ",mem_addr, 9999);
                    printf("Error: Illegal opcode; treated as 9999\n");
                }
                else{
                    if((operand%1000) >= 512){
                        printf("%03d: %04d ",mem_addr, ((operand/1000)*1000));
                        printf("Error: Absolute address exceeds machine size; zero used\n");
                    }
                    else{
                        printf("%03d: %04d\n",mem_addr, operand);
                    }
                }
                mem_addr+=1;
            }
            else if(addressmode== 'M'){
                if (operand/1000 >=10){
                    printf("%03d: %04d ",mem_addr, 9999);
                    printf("Error: Illegal opcode; treated as 9999\n");
                }
                else{
                    if((operand%1000)>=0 && (operand%1000)<j) printf("%03d: %04d\n",mem_addr, (((operand/1000)*1000)+module_add[(operand%1000)]));
                    else{
                        printf("%03d: %04d ",mem_addr, ((operand/1000)*1000));
                        printf("Error: Illegal module operand ; treated as module=0\n");
                    }
                }
                mem_addr+=1;
            }
            else if(addressmode== 'R'){
                if (operand/1000 >=10){
                    printf("%03d: %04d ",mem_addr, 9999);
                    printf("Error: Illegal opcode; treated as 9999\n");
                }
                else {
                    if((operand%1000)>instcount){
                    printf("%03d: %04d ",mem_addr, (((operand/1000)*1000) + module_add[mod]));
                    printf("Error: Relative address exceeds module size; relative zero used\n");
                    }
                    else printf("%03d: %04d\n",mem_addr, (operand + module_add[mod]));
                }
                mem_addr+=1;
            }
            else if(addressmode == 'I'){
                if (operand/1000 >=10){
                    printf("%03d: %04d ",mem_addr, 9999);
                    printf("Error: Illegal opcode; treated as 9999\n");
                }
                else{
                    if(operand%1000 >= 900){
                        printf("%03d: %04d ",mem_addr, (((operand/1000)*1000)+999));
                        printf("Error: Illegal immediate operand; treated as 999\n");
                    }
                    else printf("%03d: %04d\n",mem_addr, operand);
                }
                mem_addr+=1;
            }
            else if(addressmode == 'E'){
                if (operand/1000 >=10){
                    printf("%03d: %04d ",mem_addr, 9999);
                    printf("Error: Illegal opcode; treated as 9999\n");
                }
                else{
                    int op=operand%1000;
                    
                    int ps=-1;
                    
                    if (op>=usecount || usecount ==0){
                        
                    int a = ((operand / 1000) * 1000)+module_add[mod];
                    printf("%03d: %04d ", mem_addr, a);
                    printf("Error: External operand exceeds length of uselist; treated as relative=0\n");
                
                    }
                    else{
                        uselist_flag[op]=1;
                        if (usesymbol[op] != NULL){               
                            for (int n=0;n<symbolCount;n++){
                                if(symbolTable[n].symbol != NULL && strcmp(usesymbol[op],symbolTable[n].symbol) == 0){
                                    ps=n;
                                }
                            }
                        }
                        if (ps != -1) {  
                                int sym_val = symbolTable[ps].address;
                                deflist_flag[ps]=1;
                                int a = ((operand / 1000) * 1000) + sym_val;
                                printf("%03d: %04d\n", mem_addr, a);
                            }
                        else{
                            int a = ((operand / 1000) * 1000);
                            printf("%03d: %04d ", mem_addr, a);
                            printf("Error: %s is not defined; zero used\n",usesymbol[op]);
                        }
                    }
                }
                
                mem_addr+=1;

            }
        }
        for(int i=0;i<usecount;i++){
            if(uselist_flag[i]==0){
                printf("Warning: Module %d: uselist[%d]=%s was not used\n",mod,i,usesymbol[i]);
            }
        }
        mod++;
        for (int i = 0; i < usecount; i++) {
            free(usesymbol[i]);  
        }
        free(usesymbol);  
        free(uselist_flag);
       
    }
    return 0;
}




#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file_address>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *address = argv[1];
    buffer = malloc(buffer_size * sizeof(char));
    if (buffer == NULL) {
        perror("Failed to allocate memory for buffer");
        exit(EXIT_FAILURE);
    }
    file = fopen(address, "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    get_new_line();
    Pass1();
    printf("Symbol Table\n");
    for (int i = 0; i < symbolCount; i++) {
        if (symbol_flag[i] == 1) {
            printf("%s=%d ", symbolTable[i].symbol, symbolTable[i].address);
            printf("Error: This variable is multiple times defined; first value used\n");
        } else {
            printf("%s=%d\n", symbolTable[i].symbol, symbolTable[i].address);
        }
    }
    fseek(file, 0, SEEK_SET);
    linenum = 0;
    lineoffset = 0;

    buffer = malloc(buffer_size * sizeof(char));
    if (buffer == NULL) {
        perror("Failed to allocate memory for buffer");
        exit(EXIT_FAILURE);
    }

    printf("\nMemory Map\n");
    get_new_line();
    Pass2();

    printf("\n");
    for (int i = 0; i < symbolCount; i++) {
        if (deflist_flag[i] != 1) {
            printf("Warning: Module %d: %s was defined but never used\n", symbolTable[i].mod_n, symbolTable[i].symbol);
        }
    }
    printf("\n");

    fclose(file);
    free(buffer);

    for (int i = 0; i < symbolCount; i++) {
        free(symbolTable[i].symbol);  
    }
    free(symbolTable);  
    free(deflist_flag);
    return 0;
}

