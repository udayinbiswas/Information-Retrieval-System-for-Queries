#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <set>
#include <sstream>
#include <stack>
#include <tuple>
#include <cmath>
#include <iterator>
#include <algorithm>


using namespace std;


bool sortbysec(const tuple<int, double>& a, 
               const tuple<int, double>& b)
{
    return (get<1>(a) > get<1>(b));
}

template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

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


tuple<vector<tuple<string,string> >,vector<string> > getDocuments(vector<string> postexpression,string dictfile,string indexfile){
	stack<vector<tuple <string,string> > > stack_evaluate;
	stack<vector<string> > stack_all_evaluate;
	for (int j=0;j<postexpression.size();j++){
		if (postexpression[j]=="&" || postexpression[j]=="|"){
			vector<tuple<string,string> > docs1 = stack_evaluate.top();
			stack_evaluate.pop();
			vector<tuple<string,string> > docs2 = stack_evaluate.top();
			stack_evaluate.pop();
			vector<tuple<string,string> > finaldocs ;

			vector<string > alldocs1 = stack_all_evaluate.top();
			stack_all_evaluate.pop();
			vector<string> alldocs2 = stack_all_evaluate.top();
			stack_all_evaluate.pop();
			vector<string>  allfinaldocs ;
			
			if (postexpression[j]=="&"){
				 for (int j=0;j<docs1.size();j++){
				 	if (find(docs2.begin(), docs2.end(), docs1[j]) != docs2.end()){
				 		finaldocs.push_back(docs1[j]);
				 	}
				 }
				 for (int j=0;j<alldocs1.size();j++){
				 	if (find(alldocs2.begin(), alldocs2.end(), alldocs1[j]) != alldocs2.end()){
				 		allfinaldocs.push_back(alldocs1[j]);
				 	}
				 }
			} else if (postexpression[j]=="|"){
				finaldocs = docs2;
				for (int j=0;j<docs1.size();j++){
				 	if (find(docs2.begin(), docs2.end(), docs1[j]) == docs2.end()){
				 		finaldocs.push_back(docs1[j]);
				 	}
				 }
				 allfinaldocs = alldocs2;
				for (int j=0;j<alldocs1.size();j++){
				 	if (find(alldocs2.begin(), alldocs2.end(), alldocs1[j]) == alldocs2.end()){
				 		allfinaldocs.push_back(alldocs1[j]);
				 	}
				 }
			}
			stack_evaluate.push(finaldocs);
			stack_all_evaluate.push(allfinaldocs);
		}  else if (postexpression[j].length()>2 && postexpression[j].substr(0,2)=="N:"){
			// Named entity
			ifstream infile(dictfile);
			string word;
			while (getline(infile,word) && word!="NE"){
				//DO NOTHING
			}
			cout<<word<<endl;
			string entity = postexpression[j].substr(2,postexpression[j].length());
			if (entity.substr(entity.length()-1)=="*"){
				string prefix = entity.substr(0,entity.length()-1);
				string word;
				vector<int> offset;
				while (getline(infile,word)){
					size_t found1 = word.find_first_of(":");
					size_t found2 = word.find_last_of(":");
					string dictword =  word.substr(0,found1) ;
					string dictoffset = word.substr(found2+1) ;
					if (dictword.find(prefix) ==0){
						offset.push_back(stoi(dictoffset));
					}
				}
				infile.close();
				set<tuple<string,string> > prefixdocs;
				set<string>  allprefixdocs;
				for (int j=0;j<offset.size();j++){
					ifstream infile2(indexfile);
					int counter=0;
					string line ;
					string pl;
					while (getline(infile2,line)){
						if (counter==offset[j]){
							pl = line;
						}
						counter++;
					}
					infile2.close();
					string w;
					stringstream ss(pl);
					while (ss>>w){
						vector<std::string> x = split(w, ',');
						tuple<string,string> tup = make_tuple(x[0],x[1]);
						prefixdocs.insert(tup);
						allprefixdocs.insert(x[0]);
					}
				}
				vector<tuple<string,string> > querydoc(prefixdocs.size());
				copy(prefixdocs.begin(), prefixdocs.end(), querydoc.begin());
				stack_evaluate.push(querydoc);

				vector<string> allquerydoc(allprefixdocs.size());
				copy(allprefixdocs.begin(), allprefixdocs.end(), allquerydoc.begin());
				stack_all_evaluate.push(allquerydoc);
			} else {
				string word;
				int offset = -1;
				while (infile>>word){
					size_t found1 = word.find_first_of(":");
					size_t found2 = word.find_last_of(":");
					string dictword =  word.substr(0,found1) ;
					string dictoffset = word.substr(found2+1) ;
						if (dictword==entity){
							offset = stoi(dictoffset);
							break;
						}
				}
				infile.close();
				// Get documents for word from posting list
				string pl= "";
				ifstream infile2(indexfile);
				int counter=0;
				string line ;
				while (getline(infile2,line)){
					if (counter==offset){
						pl = line;
						break;
					}
					counter++;
				}
				infile2.close();
				vector<tuple<string,string> > docs ;
				vector<string> alldocs;
				string w;
				stringstream ss(pl);
				while (ss>>w){
					vector<std::string> x = split(w, ',');
					tuple<string,string> tup = make_tuple(x[0],x[1]);
					if (find(docs.begin(), docs.end(), tup) == docs.end()){
					 	docs.push_back(tup);
					}
					if (find(alldocs.begin(), alldocs.end(), x[0]) == alldocs.end()){
					 	alldocs.push_back(x[0]);
					}
				}
				stack_evaluate.push(docs);
				stack_all_evaluate.push(alldocs);
			}

		} else if (postexpression[j].substr(postexpression[j].length()-1)=="*"){
			string prefix = postexpression[j].substr(0,postexpression[j].length()-1);

			// See dictionary location for both query parts
			ifstream infile(dictfile);
			string word;
			vector<int> offset;
			infile>>word;
			infile>>word;
			while (infile>>word && word!="NE"){
				size_t found1 = word.find_first_of(":");
				size_t found2 = word.find_last_of(":");
				string dictword =  word.substr(0,found1) ;
				string dictoffset = word.substr(found2+1) ;
				if (dictword.find(prefix) ==0){
					offset.push_back(stoi(dictoffset));
				}
			}
			infile.close();
			set<tuple<string,string> > prefixdocs;
			set<string> allprefixdocs;
			for (int j=0;j<offset.size();j++){
				ifstream infile2(indexfile);
				int counter=0;
				string line ;
				string pl;
				while (getline(infile2,line)){
					if (counter==offset[j]){
						pl = line;
					}
					counter++;
				}
				infile2.close();
				string w;
				stringstream ss(pl);
				while (ss>>w){
					vector<std::string> x = split(w, ',');
					tuple<string,string> tup = make_tuple(x[0],x[1]);
					prefixdocs.insert(tup);
					allprefixdocs.insert(x[0]);
				}
			}
			vector<tuple<string,string> > querydoc(prefixdocs.size());
			copy(prefixdocs.begin(), prefixdocs.end(), querydoc.begin());
			stack_evaluate.push(querydoc);

			vector<string>  allquerydoc(allprefixdocs.size());
			copy(allprefixdocs.begin(), allprefixdocs.end(), allquerydoc.begin());
			stack_all_evaluate.push(allquerydoc);
		}   else {
			// stack_evaluate.push(postexpression[j]);
			// See dictionary location for both query parts
			ifstream infile(dictfile);
			string word;
			int offset = -1;
			infile>>word;
			infile>>word;
			while (infile>>word && word!="NE"){
				size_t found1 = word.find_first_of(":");
				size_t found2 = word.find_last_of(":");
				string dictword =  word.substr(0,found1) ;
				string dictoffset = word.substr(found2+1) ;
					if (dictword==postexpression[j]){
						offset = stoi(dictoffset);
						break;
					}
			}
			infile.close();
			// Get documents for word from posting list
			string pl= "";
			ifstream infile2(indexfile);
			int counter=0;
			string line ;
			while (getline(infile2,line)){
				if (counter==offset){
					pl = line;
					break;
				}
				counter++;
			}
			infile2.close();
			vector<tuple<string,string> > docs ;
			vector<string> allDocs;
			string w;
			stringstream ss(pl);
			while (ss>>w){
				vector<std::string> x = split(w, ',');
				tuple<string,string> tup = make_tuple(x[0],x[1]);
				if (find(docs.begin(), docs.end(), tup) == docs.end()){
				 	docs.push_back(tup);
				}
				if (find(allDocs.begin(), allDocs.end(), x[0]) == allDocs.end()){
				 	allDocs.push_back(x[0]);
				}
				
			}
			stack_evaluate.push(docs);
			stack_all_evaluate.push(allDocs);
		}
	}
	// done
	vector<tuple<string,string> > querydocs;
	if (!stack_evaluate.empty()){
	querydocs = stack_evaluate.top();
	}

	vector<string>  allquerydocs;
	if (!stack_all_evaluate.empty()){
	allquerydocs = stack_all_evaluate.top();
	}
	return make_tuple(querydocs,allquerydocs);
}


vector<tuple<int,double> > getDocScores(vector<tuple<string,string> > v){
	map<int,set<int> > docparas;
	for (int i=0;i<v.size();i++){
		tuple<string,string> t = v[i];
		int doc = stoi(get<0>(t));
		int para = stoi(get<1>(t));
		docparas[doc].insert(para);
	}
	vector<tuple<int,double> > scores;
	for (map<int,set<int> >::iterator i=docparas.begin(); i!=docparas.end(); ++i){
		double score = 0;
    	for (set<int> ::iterator j=i->second.begin();j!=i->second.end();++j){
    		double h = pow(double(2),double(*j));
    		score+= 1/h;
    	}
    	scores.push_back(make_tuple(i->first,score));
    }
    return scores;
}


int main(int argc, char *argv[]){
	string queryfile = "" ;
	string cutoff = "100" ;
	string resultfile = "";
	string indexfile = "";
	string dictfile = "";
	bool collpathmen = false;

	for (int i=1;i<argc;i++){
		string argument = argv[i];
		if (argument=="--query"){
			queryfile = argv[i+1];
		} else if (argument=="--cutoff"){
			cutoff = argv[i+1];
		} else if (argument=="--output"){
			resultfile = argv[i+1];
		} else if (argument=="--index"){
			indexfile = argv[i+1];
		} else if (argument=="--dict"){
			dictfile = argv[i+1];
		} 
	}
	cout<<queryfile<<","<<cutoff<<","<<resultfile<<","<<indexfile<<","<<dictfile<<endl;
	// Read queries
	string line;
	string word;
	vector<string> queries;
	vector<string> filenames;
	string stemming = "0";
	string stopword = "0";
	ifstream incond(dictfile);
	incond>>stopword;
	incond>>stemming;
	incond.close();
	// string docpath = "query.txt";
	// string preprocess = "python preprocess.py "+docpath+" "+stopword+" "+stemming;
	// int res = system(preprocess.c_str());
	ifstream infile(queryfile);
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
	ofstream outfile(resultfile);
	for (int i=0;i<queries.size();i++){
		//For each query, do the preprocessing required 
		string query = queries[i];
		string modquery = modifyQuery(query);
		ofstream putppquery("ppquery.txt");
		putppquery<<modquery;
		putppquery.close();
		string preprocess = "python ppqueries.py ppquery.txt "+stopword+" "+stemming;
		int res = system(preprocess.c_str());
		ifstream getppquery("ppquery.txt");
		getline(getppquery,modquery);
		getppquery.close();
		// Preprocessing done
		stringstream ss(modquery);
		vector<string> op;
		string word;
		while (ss>>word){
			op.push_back(word);
		}
		// Infix to Postfix query
		vector<string> postexpression = intopost(op);
		// Return the documents for the query
		tuple<vector<tuple<string,string> >,vector<string> > t = getDocuments(postexpression,dictfile,indexfile);
		vector<tuple<string,string> > querydocs = get<0>(t);		
		vector<tuple<int,double> > topdocs = getDocScores(querydocs);
		sort(topdocs.begin(), topdocs.end(), sortbysec);
		vector<string> allquerydocs = get<1>(t);


		vector<int> topdocspresent;
		for (int j=0;j<topdocs.size();j++){
			topdocspresent.push_back(get<0>(topdocs[j]));
		}

		vector<tuple<int,double> > finaldocs = topdocs;
		for (int j=0;j<allquerydocs.size();j++){
			if (find(topdocspresent.begin(), topdocspresent.end(), stoi(allquerydocs[j])) == topdocspresent.end()){
				 		finaldocs.push_back(make_tuple(stoi(allquerydocs[j]),0));
			}
		}
		for (int k=0;k<min(stoi(cutoff),int(finaldocs.size()));k++){
			outfile<<filenames[get<0>(finaldocs[k])]<<" "<<get<1>(finaldocs[k])<<"\n";
		}
		outfile<<"\n\n";
	}
	outfile.close();
	return 0;
}