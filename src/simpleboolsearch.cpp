#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <set>
#include <sstream>
#include <stack>

using namespace std;
// left to right precedence, | & ( )
string modifyQuery(string query){
	string modquery ="";
	for (int j=0;j<query.length();j++){
		if (query.at(j)=='('){
			modquery += query.at(j);
			modquery += " ";
		} else if (query.at(j)==')'){
			modquery += " ";
			modquery += query.at(j);
		} else {
			modquery += query.at(j);
		}
	}
	return modquery;
}

vector<string> intopost(vector<string> v){
	vector<string> output;
	stack<string> stacked;

	for (int i=0;i<v.size();i++){
		if (v[i]=="("){
			stacked.push(v[i]);
		} else if (v[i]==")"){
			while (stacked.top()!="("){
				output.push_back(stacked.top());
				stacked.pop();
			}
			stacked.pop(); //pop out (
		} else if (v[i]=="&" || v[i]=="|"){
			if (stacked.empty()){
				stacked.push(v[i]);
			} else if (stacked.top()!="&" && stacked.top()!="|"){
				stacked.push(v[i]);
			} else {
				string s = stacked.top();
				stacked.pop();
				stacked.push(v[i]);
				output.push_back(s);
			}
		} else {
			output.push_back(v[i]);
		}
	}
	while (stacked.size()!=0){
		string s = stacked.top();
		output.push_back(s);
		stacked.pop();
	}
	return output;
}


vector<string> getDocuments(vector<string> postexpression){
	stack<vector<string> > stack_evaluate;
	for (int j=0;j<postexpression.size();j++){
		if (postexpression[j]=="&" || postexpression[j]=="|"){
			vector<string> docs1 = stack_evaluate.top();
			stack_evaluate.pop();
			vector<string> docs2 = stack_evaluate.top();
			stack_evaluate.pop();
			
			vector<string> finaldocs ;
			if (postexpression[j]=="&"){
				 for (int j=0;j<docs1.size();j++){
				 	if (find(docs2.begin(), docs2.end(), docs1[j]) != docs2.end()){
				 		finaldocs.push_back(docs1[j]);
				 	}
				 }
			} else if (postexpression[j]=="|"){
				finaldocs = docs2;
				for (int j=0;j<docs1.size();j++){
				 	if (find(docs2.begin(), docs2.end(), docs1[j]) == docs2.end()){
				 		finaldocs.push_back(docs1[j]);
				 	}
				 }
			}
			stack_evaluate.push(finaldocs);
		}  else {
			// stack_evaluate.push(postexpression[j]);
			// See dictionary location for both query parts
			ifstream infile("indexfile.dict");
			string word;
			int offset = -1;
			while (infile>>word){
				size_t found1 = word.find_first_of(":");
				size_t found2 = word.find_last_of(":");
				string dictword =  word.substr(0,found1) ;
					string dictoffset = word.substr(found2+1) ;
					if (dictword==postexpression[j]){
						offset = stoi(dictoffset);
					}
			}
			infile.close();
			// Get documents for word from posting list
			string pl= "";
			ifstream infile2("indexfile.idx");
			int counter=0;
			string line ;
			while (getline(infile2,line)){
				if (counter==offset){
					pl = line;
				}
				counter++;
			}
			infile2.close();
			vector<string> docs ;
			string w;
			stringstream ss(pl);
			while (ss>>w){
				docs.push_back(w);
			}
			stack_evaluate.push(docs);
		}
	}
	// done
	vector<string> querydocs;
	if (!stack_evaluate.empty()){
	querydocs = stack_evaluate.top();
	}
	return querydocs;
}

int main(int argc, char *argv[]){
	// Read queries
	string line;
	string word;
	vector<string> queries;
	vector<string> filenames;
	ifstream infile("query.txt");
	while (getline(infile,line)){
		queries.push_back(line);
	}
	infile.close();
	ifstream infile2("original_files.txt");
	while (infile2>>word){
		filenames.push_back(word);
	}
	infile2.close();
	// Perform for each query
	ofstream outfile("results.txt");
	for (int i=0;i<queries.size();i++){
		//For each query, do the parsing 
		string query = queries[i];
		string modquery = modifyQuery(query);
		stringstream ss(modquery);
		vector<string> op;
		string word;
		while (ss>>word){
			op.push_back(word);
		}
		// Infix to Postfix query
		vector<string> postexpression = intopost(op);
		// Return the documents for the query
		vector<string> querydocs = getDocuments(postexpression);
		for (int k=0;k<querydocs.size();k++){
			outfile<<filenames[stoi(querydocs[k])]<<"\n";
		}
		outfile<<"\n";
	}
	outfile.close();
	return 0;
}