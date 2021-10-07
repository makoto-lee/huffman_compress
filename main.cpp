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

string raw_head_file = "";					// �Ψ��x�shead file��raw data (for �����Y)
unordered_map<char, int> freq;				// �ΨӲέp�U�Ӧr�����X�{����
deque<Node*> forest;						// �Ψ��x�s�U�Ӹ`�I������
unordered_map<char, string> char_code;		// �Ψ��x�s�C�Ӧr���s�X�L�᪺�s�X
Node root;									// �ΨӰO�������Y��Ū�쪺��
char max_code_len = 0;						// �����s�X���̪�����
char last_byte = 0;							// �ΨӰO���̫�@��byte�u���δX��bit
unsigned long long source_file_size = 0;	// �ΨӰO�����ɮפj�p(byte��)
unsigned long long compressed_file_size = 0;// �ΨӰO�����Y�ɤj�p(byte��)

bool comp(Node* a, Node* b) {				// ���ѵ�sort��ƨϥ�
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

void make_char_code(const char *filename) {		// ���J���ɮץͦ�char_code
	fstream file_in(filename, ios::in | ios::binary);
	while (file_in.peek() != EOF) {		//Ū�J�ɮפ�����ơA�òέp�U�r�����X�{�W�v
		char c;
		c = file_in.get();
		if (freq.count(c)) freq[c]++;
		else freq[c] = 1;
		source_file_size++;
	}
	// �s�@�`�I�A���x�s��forest����
	for (auto it = freq.begin(); it != freq.end(); it++) {
		forest.push_back(new Node((*it).first, (*it).second, nullptr, nullptr));
	}

	// ���Ӧr���X�{�W�v�Ƨ�forest�����`�I�A���X�X�{�W�v�̤p����Ӹ`�I�A�N��X�֡A
	// �A�x�s�^forest����
	// �C�����|�����ƧǡA�A�X��
	for (int i = 0; i < freq.size() - 1; i++) {
		sort(forest.begin(), forest.end(), comp);
		Node* ptr1 = forest.front(); forest.pop_front();
		Node* ptr2 = forest.front(); forest.pop_front();
		int new_val = (ptr1->val < ptr2->val ? ptr1->val : ptr2->val);		// �N���`�I���ȳ]���r��Ǹ��p���l�`�I����
		if (ptr1->val > ptr2->val) swap(ptr1, ptr2);
		Node* parentNode = new Node(new_val, ptr1->freq + ptr2->freq, ptr1, ptr2);
		forest.push_back(parentNode);
	}
	// �s�X
	Code(forest.front(), string(""));

	file_in.close();
}

void print_char_in_binary(char input) {		//�N�ǤJ��char�H0��1 print�X(������)
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
	//�@�Ƿ|���ƥΨ쪺�ܼ�
	char now_write = NULL;
	string str_tmp;
	int char_full = 0;
	//�g�Jfile header(�h��bit���@��)
	compressed_file_size = 2;
	file_out.write(&max_code_len, 1);
	file_out.seekp(2);	// ���L��1��
	// �g�J�s�X(*o*) 256��
	unordered_map<char, string>::const_iterator got;
	for (int i = 0; i < 256; i++) {
		got = char_code.find(i);
		if (got != char_code.end()) {	// �Ӧr�����X�{�L
			str_tmp = got->second;
			code_patch(str_tmp);
		}
		else							// �Ӧr���S���X�{�L
			str_tmp = empty_code();
		while (str_tmp != "") {
			now_write = now_write << 1;
			now_write = now_write + ((str_tmp[0] == '1') ? 1 : 0);
			str_tmp.erase(str_tmp.begin());	//����str_tmp���Ĥ@�Ӧr
			char_full++;
			if (char_full == 8) {
				file_out.write(&now_write, 1);		//1 = sizeof(char)
				compressed_file_size++;
				now_write = NULL;
				char_full = 0;
			}
		}
	}
	//�̫�p�G������8bit���s�X�A�ɹs��J
	if (char_full != 0) {
		now_write = now_write << (8 - char_full);
		//print_char_in_binary(now_write);
		file_out.write(&now_write, 1);
		compressed_file_size++;
	}
	// �q�Y���sŪ���ɮסA�ýs�X�A�üg�J���Y��
	cout << "/==========" << endl;
	string wait;	now_write = NULL;	char_full = 0;
	while (file_in.peek() != EOF) {
		char c = file_in.get();
		wait = char_code[c];		//��wait�s�Jc�r�����s�X
		while (wait != "") {
			now_write = now_write << 1;
			now_write = now_write + ((wait[0] == '1') ? 1 : 0);
			wait.erase(wait.begin());	//����wait���Ĥ@�Ӧr
			char_full++;
			if (char_full == 8) {
				file_out.write(&now_write, 1);		//1 = sizeof(char)
				compressed_file_size++;
				now_write = NULL;
				char_full = 0;
			}
		}

	}
	//�̫�p�G������8bit���s�X�A�ɹs��J
	if (char_full != 0) {
		now_write = now_write << (8 - char_full);
		file_out.write(&now_write, 1);
		compressed_file_size++;
	}
	//�^���ɮת���1��g�J
	file_out.seekp(1);
	last_byte = (char)char_full;
	file_out.write(&last_byte, 1);	// �̫�@��byte�ΤF�h��bit
	//�ͦ��ɮ����Y����
	file_in.close();
	file_out.close();
}

string char_to_string(char input) {			// �ǤJ��char�নstring ��X
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

void make_map() {		// ��raw head file(string) �ͦ��U�Ӧr����unorder map
	string empty = empty_code();		// �Ω����O�_���Ū��s�X
	char c = NULL;						// �Ω�����g�Junorder map����}
	for (int i = 0; i < 256; i++) {
		string read;
		read.assign(raw_head_file, 0, max_code_len + 1);
		raw_head_file.erase(0, max_code_len + 1);
		if (read == empty) { c++; continue; }	// Ū��Ū��s�X
		// ���h�q�̫�ƨӪ��Ĥ@��'1'
		while (read.back() == '0')
			read.erase(read.end() - 1);
		read.erase(read.end() - 1);
		// �g�Jchar_code============
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

void char_to_bool(bool *barr, char c, int bits) {			// �N�ǤJ��char �̷Ө�8��bit���� ,�s�b�@��8�檺bool�}�C
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
	// Ū��˼Ʋ�2��btye
	while (true) {

		// �ˬd�U�@�Ӧr���O�_�� EOF
		file_in.seekg(1, ios::cur);
		if (file_in.peek() == EOF) break;
		file_in.seekg(-1, ios::cur);
		// ==========
		curr = file_in.get();

		//print_char_in_binary(curr);
		char_to_bool(barr, curr, 8);
		for (int i = 0; i < 8; i++) {
			// �̷�Ū���쪺1 or 0 node ������
			if (barr[i])
				curr_node = curr_node->rchild;
			else
				curr_node = curr_node->lchild;
			// ����Fleaf node
			if (curr_node->lchild == NULL || curr_node->rchild == NULL) {
				file_out.write(&(curr_node->val), 1);
				curr_node = &root;		// �^root
			}
		}
	}
	// Ū�̫�@�Ӧr��
	file_in.seekg(-1, ios::end);
	curr = file_in.get();
	//print_char_in_binary(curr);
	char_to_bool(barr, curr, (int)last_byte);
	for (int i = 0; i < (int)last_byte; i++) {
		// node �̷�Ū���쪺 1 or 0 ������
		if (barr[i])
			curr_node = curr_node->rchild;
		else
			curr_node = curr_node->lchild;
		// ����Fleaf node
		if (curr_node->lchild == NULL || curr_node->rchild == NULL) {
			file_out.write(&(curr_node->val), 1);
			curr_node = &root;		// �^root
		}
	}
	// ����
	file_in.close();
	file_out.close();
}

void uncompress_file(const char *sourse, const char *output) {
	fstream file_in(sourse, ios::in | ios::binary);
	max_code_len = file_in.get();
	last_byte = file_in.get();
	int read_time = (max_code_len + 1) * 256 / 8;
	char char_tmp = 0;
	// Ū��head file
	for (int i = 0; i < read_time; i++) {
		char_tmp = file_in.get();
		string str = char_to_string(char_tmp);
		raw_head_file = raw_head_file + str;
	}
	file_in.close();
	// �_��file����huffman tree
	make_map();
	make_tree();
	// �}�l�ѽX
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
				// ���Y
				make_char_code(argv[i_index]);
				make_output_file(argv[i_index], argv[o_index]);
				printf("Input File Size :\t%d(bytes)\n", source_file_size);
				printf("Output File Size :\t%d(bytes)\n", compressed_file_size);
				printf("Compress Rate :\t%.2f%%\n", ((double)compressed_file_size / source_file_size) * 100);
			}
			if (*(argv[1] + 1) == 'u') {
				// �����Y
				uncompress_file(argv[i_index], argv[o_index]);
			}
		}
	}
}