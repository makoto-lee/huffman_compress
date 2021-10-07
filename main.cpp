#include<iostream>
#include <deque>
#include <string>
#include <unordered_map>
#include<algorithm>
#include<fstream>
using namespace std;

class Node {
public:
	char val;
	int freq;
	Node* lchild;
	Node* rchild;

	Node(char _val = NULL, int _freq = NULL, Node* _lchild = NULL, Node* _rchild = NULL) {
		val = _val;
		freq = _freq;
		lchild = _lchild;
		rchild = _rchild;
	}
};

string raw_head_file = "";					// 用來儲存head file的raw data (for 解壓縮)
unordered_map<char, int> freq;				// 用來統計各個字元的出現次數
deque<Node*> forest;						// 用來儲存各個節點之指標
unordered_map<char, string> char_code;		// 用來儲存每個字元編碼過後的編碼
Node root;									// 用來記錄解壓縮時讀到的樹
char max_code_len = 0;						// 紀錄編碼的最長長度
char last_byte = 0;							// 用來記錄最後一個byte只有用幾個bit
unsigned long long source_file_size = 0;	// 用來記錄原檔案大小(byte數)
unsigned long long compressed_file_size = 0;// 用來記錄壓縮檔大小(byte數)

bool comp(Node* a, Node* b) {				// 提供給sort函數使用
	return (a->freq < b->freq ? true : (a->freq > b->freq ? false : a->val < b->val));
}

void Code(Node* ptr, string s) {
	if (ptr->lchild == nullptr || ptr->rchild == nullptr) {
		printf("'%c' (frequence = %d) --> %s\n", ptr->val, freq[ptr->val], s.c_str());
		char_code[ptr->val] = s;
		if (max_code_len < s.length()) max_code_len = s.length();
		return;
	}
	if (ptr->lchild) Code(ptr->lchild, string(s + "0"));
	if (ptr->rchild) Code(ptr->rchild, string(s + "1"));
}

void make_char_code(const char *filename) {		// 對輸入的檔案生成char_code
	fstream file_in(filename, ios::in | ios::binary);
	while (file_in.peek() != EOF) {		//讀入檔案內的資料，並統計各字元的出現頻率
		char c;
		c = file_in.get();
		if (freq.count(c)) freq[c]++;
		else freq[c] = 1;
		source_file_size++;
	}
	// 製作節點，並儲存到forest之中
	for (auto it = freq.begin(); it != freq.end(); it++) {
		forest.push_back(new Node((*it).first, (*it).second, nullptr, nullptr));
	}

	// 按照字元出現頻率排序forest中的節點，取出出現頻率最小的兩個節點，將其合併，
	// 再儲存回forest之中
	// 每次都會先做排序，再合併
	for (int i = 0; i < freq.size() - 1; i++) {
		sort(forest.begin(), forest.end(), comp);
		Node* ptr1 = forest.front(); forest.pop_front();
		Node* ptr2 = forest.front(); forest.pop_front();
		int new_val = (ptr1->val < ptr2->val ? ptr1->val : ptr2->val);		// 將父節點的值設為字典序較小的子節點的值
		if (ptr1->val > ptr2->val) swap(ptr1, ptr2);
		Node* parentNode = new Node(new_val, ptr1->freq + ptr2->freq, ptr1, ptr2);
		forest.push_back(parentNode);
	}
	// 編碼
	Code(forest.front(), string(""));

	file_in.close();
}

void print_char_in_binary(char input) {		//將傳入的char以0跟1 print出(除錯用)
	typedef union {
		char input;
		struct {
			unsigned int H : 1;
			unsigned int G : 1;
			unsigned int F : 1;
			unsigned int E : 1;
			unsigned int D : 1;
			unsigned int C : 1;
			unsigned int B : 1;
			unsigned int A : 1;
		};
	}INPUT;
	INPUT sep;
	sep.input = input;
	cout << sep.A << sep.B << sep.C << sep.D << sep.E << sep.F << sep.G << sep.H << endl;
}

void code_patch(string &input) {
	input = input + "1";
	while (input.length() != max_code_len + 1)
		input = input + "0";
}

string empty_code() {
	string re;
	for (int i = 0; i < max_code_len + 1; i++)
		re = re + "0";
	return re;
}

void make_output_file(const char *sourse, const char *output) {
	fstream file_in(sourse, ios::in | ios::binary);
	fstream file_out(output, ios::out | ios::binary);
	//一些會重複用到的變數
	char now_write = NULL;
	string str_tmp;
	int char_full = 0;
	//寫入file header(多少bit為一組)
	compressed_file_size = 2;
	file_out.write(&max_code_len, 1);
	file_out.seekp(2);	// 跳過第1格
	// 寫入編碼(*o*) 256個
	unordered_map<char, string>::const_iterator got;
	for (int i = 0; i < 256; i++) {
		got = char_code.find(i);
		if (got != char_code.end()) {	// 該字元有出現過
			str_tmp = got->second;
			code_patch(str_tmp);
		}
		else							// 該字元沒有出現過
			str_tmp = empty_code();
		while (str_tmp != "") {
			now_write = now_write << 1;
			now_write = now_write + ((str_tmp[0] == '1') ? 1 : 0);
			str_tmp.erase(str_tmp.begin());	//擦掉str_tmp的第一個字
			char_full++;
			if (char_full == 8) {
				file_out.write(&now_write, 1);		//1 = sizeof(char)
				compressed_file_size++;
				now_write = NULL;
				char_full = 0;
			}
		}
	}
	//最後如果有不足8bit的編碼，補零輸入
	if (char_full != 0) {
		now_write = now_write << (8 - char_full);
		//print_char_in_binary(now_write);
		file_out.write(&now_write, 1);
		compressed_file_size++;
	}
	// 從頭重新讀取檔案，並編碼，並寫入壓縮檔
	cout << "/==========" << endl;
	string wait;	now_write = NULL;	char_full = 0;
	while (file_in.peek() != EOF) {
		char c = file_in.get();
		wait = char_code[c];		//給wait存入c字元的編碼
		while (wait != "") {
			now_write = now_write << 1;
			now_write = now_write + ((wait[0] == '1') ? 1 : 0);
			wait.erase(wait.begin());	//擦掉wait的第一個字
			char_full++;
			if (char_full == 8) {
				file_out.write(&now_write, 1);		//1 = sizeof(char)
				compressed_file_size++;
				now_write = NULL;
				char_full = 0;
			}
		}

	}
	//最後如果有不足8bit的編碼，補零輸入
	if (char_full != 0) {
		now_write = now_write << (8 - char_full);
		file_out.write(&now_write, 1);
		compressed_file_size++;
	}
	//回到檔案的第1格寫入
	file_out.seekp(1);
	last_byte = (char)char_full;
	file_out.write(&last_byte, 1);	// 最後一個byte用了多少bit
	//生成檔案壓縮完成
	file_in.close();
	file_out.close();
}

string char_to_string(char input) {			// 傳入的char轉成string 輸出
	char tmp = input;
	string str;
	for (int i = 0; i < 8; i++) {
		if (tmp < 0)
			str = str + '1';
		else
			str = str + '0';
		tmp = tmp << 1;
	}
	return str;
}

void make_map() {		// 用raw head file(string) 生成各個字元的unorder map
	string empty = empty_code();		// 用於比較是否為空的編碼
	char c = NULL;						// 用於紀錄寫入unorder map的位址
	for (int i = 0; i < 256; i++) {
		string read;
		read.assign(raw_head_file, 0, max_code_len + 1);
		raw_head_file.erase(0, max_code_len + 1);
		if (read == empty) { c++; continue; }	// 讀到空的編碼
		// 除去從最後數來的第一個'1'
		while (read.back() == '0')
			read.erase(read.end() - 1);
		read.erase(read.end() - 1);
		// 寫入char_code============
		char_code[c++] = read;
	}
}

void make_tree() {
	for (int i = 0; i < 256; i++) {
		if (char_code[i].empty()) continue;
		string curr_str = char_code[i];
		Node *curr_node = &root;
		for (int i = 0; i < curr_str.size(); i++) {
			if (curr_str[i] == '1') {
				if (curr_node->rchild == NULL)
					curr_node->rchild = new Node();
				curr_node = curr_node->rchild;
			}
			else {
				if (curr_node->lchild == NULL)
					curr_node->lchild = new Node();
				curr_node = curr_node->lchild;
			}
		}
		curr_node->val = (char)i;
	}
}

void char_to_bool(bool *barr, char c, int bits) {			// 將傳入的char 依照其8個bit的值 ,存在一個8格的bool陣列
	char tmp = c;
	for (int i = 0; i < bits; i++) {
		if (tmp < 0)
			barr[i] = 1;
		else
			barr[i] = 0;
		tmp = tmp << 1;
	}
}

void decode_file(const char *sourse, const char *output) {
	fstream file_in(sourse, ios::in | ios::binary);
	fstream file_out(output, ios::out | ios::binary);
	int read_pos = 2 + (max_code_len + 1) * 256 / 8;
	file_in.seekg(read_pos, ios::cur);
	char curr;
	bool barr[8];
	Node *curr_node = &root;
	// 讀到倒數第2個btye
	while (true) {

		// 檢查下一個字元是否為 EOF
		file_in.seekg(1, ios::cur);
		if (file_in.peek() == EOF) break;
		file_in.seekg(-1, ios::cur);
		// ==========
		curr = file_in.get();

		//print_char_in_binary(curr);
		char_to_bool(barr, curr, 8);
		for (int i = 0; i < 8; i++) {
			// 依照讀取到的1 or 0 node 做移動
			if (barr[i])
				curr_node = curr_node->rchild;
			else
				curr_node = curr_node->lchild;
			// 走到了leaf node
			if (curr_node->lchild == NULL || curr_node->rchild == NULL) {
				file_out.write(&(curr_node->val), 1);
				curr_node = &root;		// 回root
			}
		}
	}
	// 讀最後一個字元
	file_in.seekg(-1, ios::end);
	curr = file_in.get();
	//print_char_in_binary(curr);
	char_to_bool(barr, curr, (int)last_byte);
	for (int i = 0; i < (int)last_byte; i++) {
		// node 依照讀取到的 1 or 0 做移動
		if (barr[i])
			curr_node = curr_node->rchild;
		else
			curr_node = curr_node->lchild;
		// 走到了leaf node
		if (curr_node->lchild == NULL || curr_node->rchild == NULL) {
			file_out.write(&(curr_node->val), 1);
			curr_node = &root;		// 回root
		}
	}
	// 關檔
	file_in.close();
	file_out.close();
}

void uncompress_file(const char *sourse, const char *output) {
	fstream file_in(sourse, ios::in | ios::binary);
	max_code_len = file_in.get();
	last_byte = file_in.get();
	int read_time = (max_code_len + 1) * 256 / 8;
	char char_tmp = 0;
	// 讀取head file
	for (int i = 0; i < read_time; i++) {
		char_tmp = file_in.get();
		string str = char_to_string(char_tmp);
		raw_head_file = raw_head_file + str;
	}
	file_in.close();
	// 復原file中的huffman tree
	make_map();
	make_tree();
	// 開始解碼
	decode_file(sourse, output);
}

int main(int argc, char *argv[]) {
	int i_index, o_index;
	if (argc == 6) {
		for (int i = 0; i < 6; i++) {
			if (*argv[i] == '-') {
				if (*(argv[i] + 1) == 'i')i_index = i + 1;
				if (*(argv[i] + 1) == 'o')o_index = i + 1;
			}
		}
		if (*argv[1] == '-') {
			if (*(argv[1] + 1) == 'c') {
				// 壓縮
				make_char_code(argv[i_index]);
				make_output_file(argv[i_index], argv[o_index]);
				printf("Input File Size :\t%d(bytes)\n", source_file_size);
				printf("Output File Size :\t%d(bytes)\n", compressed_file_size);
				printf("Compress Rate :\t%.2f%%\n", ((double)compressed_file_size / source_file_size) * 100);
			}
			if (*(argv[1] + 1) == 'u') {
				// 解壓縮
				uncompress_file(argv[i_index], argv[o_index]);
			}
		}
	}
}