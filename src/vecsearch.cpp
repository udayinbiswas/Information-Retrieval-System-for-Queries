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
			modquery += " ";
		} else if (query.at(j)==')'){
			modquery += " ";
		} else if (query.at(j)=='&'){
			modquery += " ";
		} else if (query.at(j)=='|'){
			modquery += " ";
		} else {
			modquery += query.at(j);
		}
	}
	return modquery;
}




vector<tuple<int,double> > getDocuments(map<int,map<string,double> > query,double queryNorm,int numDocs,string dictfile,string indexfile){
	//Query is disjunctive
	double getDotProduct = 0;
	map<int,double> v;
	for (map<int,map<string,double> >::iterator i = query.begin();i!=query.end();i++){
		int isneterm = i->first;
		for (map<string,double>::iterator j= i->second.begin();j!=i->second.end();j++){
			if (isneterm==0){
				// double wq = (1+(log(query[queryword])/log(2)))*(log(1+numDocs/df)/log(2));
				string queryword = j->first;
				double wq = j->second;
				ifstream infile("termfrequencies.txt");
				string a1,a2,a3;
				while (infile>>a1>>a2>>a3){
					if (a2==queryword){
						// dot product of 2 elements
						v[stoi(a1)]+= (stod(a3)*wq);
					}
				}
				infile.close();
			} else {
				string queryword = j->first;
				double wq = j->second;
				ifstream infile("netermfrequencies.txt");
				string line;
				while (getline(infile,line)){
					size_t found1 = line.find_first_of(":");
					size_t found2 = line.find_last_of(":");
					string a1 =  line.substr(0,found1) ;
					string a2 = line.substr(found1+1,found2) ;
					string a3 = line.substr(found2+1) ;
					if (a2==queryword){
						// dot product of 2 elements
						v[stoi(a1)]+= (stod(a3)*wq);
					}
				}
				infile.close();
			}
		}
	}
	map<int,double> normval;
	string docidentity;
	string wdoc;
	ifstream infile("norms.txt");
	while (infile>>docidentity>>wdoc){
		normval[stoi(docidentity)] = stod(wdoc);
	}
	infile.close();
	vector<tuple<int,double> > finalscores;
	for (map<int,double>::iterator i=v.begin();i!=v.end();i++){
		int doc = i->first;
		// cout<<doc<<","<<i->second<<","<<normval[doc]<<","<<queryNorm<<endl;
		finalscores.push_back(make_tuple(doc,(i->second)/(normval[doc]*queryNorm)));
	}
	sort(finalscores.begin(), finalscores.end(), sortbysec);
	return finalscores;
}



double computeNorm(map<int,map<string,double> > expression,int numDocs){
	double norm = 0;
	for (map<int,map<string,double> >::iterator i=expression.begin();i!=expression.end();i++){
		for (map<string,double>::iterator j= i->second.begin();j!=i->second.end();j++){
			double freq = j->second;
			norm+= pow(freq,2);
		}
	}
	norm = sqrt(norm);
	return norm;
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
	// cout<<queryfile<<","<<cutoff<<","<<resultfile<<","<<indexfile<<","<<dictfile<<endl;
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
	int numDocs = filenames.size();
	for (int i=0;i<queries.size();i++){
		//For each query, do the preprocessing required 
		string query = queries[i];
		string modquery = modifyQuery(query);
		ofstream putppquery("ppquery.txt");
		putppquery<<modquery;
		putppquery.close();
		string preprocess = "python vecppqueries.py ppquery.txt "+stopword+" "+stemming;
		int res = system(preprocess.c_str());
		ifstream getppquery("ppquery.txt");
		getline(getppquery,modquery);
		getppquery.close();
		cout<<"Query:"<<modquery<<endl;
		// Preprocessing done
		stringstream ss(modquery);
		map<int,map<string,double> > expression;
		string word;
		while (ss>>word){
			if (word.length()>2 && word.substr(0,2)=="N:"){
				ifstream infile(dictfile);
				string w;
				while (getline(infile,w) && w!="NE"){
					//DO NOTHING
				}
				string entity = word.substr(2,word.length());
				if (entity.substr(entity.length()-1)=="*"){
					string prefix = entity.substr(0,entity.length()-1);
					string w;
					vector<int> offset;
					while (getline(infile,w)){
						size_t found1 = w.find_first_of(":");
						size_t found2 = w.find_last_of(":");
						string dictword =  w.substr(0,found1) ;
						string dictfreq = (w.substr(found1+1,found2)) ;
						if (dictword.find(prefix) ==0){
							expression[1][dictword]+= (1*(log(1+numDocs/stoi(dictfreq))/log(2)));
							// expression[1][dictword]+=1;
						}
					}
					infile.close();
				} else {
					string w;
					int offset = -1;
					while (infile>>w){
						size_t found1 = w.find_first_of(":");
						size_t found2 = w.find_last_of(":");
						string dictword =  w.substr(0,found1) ;
						string dictfreq = (w.substr(found1+1,found2)) ;
							if (dictword==entity){
								expression[1][dictword]+= (1*(log(1+numDocs/stoi(dictfreq))/log(2)));
								// expression[1][dictword]+=1;
							}
					}
					infile.close();
				}
			} else if (word.substr(word.length()-1)=="*"){
				string prefix = word.substr(0,word.length()-1);
				string w;
				ifstream infile(dictfile);
				while (infile>>w && w!="NE"){
					size_t found1 = w.find_first_of(":");
					size_t found2 = w.find_last_of(":");
					string dictword =  w.substr(0,found1) ;
					string dictfreq = (w.substr(found1+1,found2)) ;
					if (dictword.find(prefix) ==0){
						expression[0][dictword]+= (1*(log(1+numDocs/stoi(dictfreq))/log(2)));
						// expression[1][dictword]+=1;
					}
				}
				infile.close();
			} else {
				string w;
				ifstream infile(dictfile);
				while (infile>>w && w!="NE"){

					size_t found1 = w.find_first_of(":");
					size_t found2 = w.find_last_of(":");
					string dictword =  w.substr(0,found1) ;
					string dictfreq = (w.substr(found1+1,found2)) ;
					if (dictword==word){
						expression[0][dictword]+= (1*(log(1+numDocs/stoi(dictfreq))/log(2)));
						// expression[1][dictword]+=1;
						break;
					}
				}
				infile.close();
			}
		}
		// Compute the norm of the query
		double querynorm = computeNorm(expression,numDocs);
		// Return the documents for the query
		vector<tuple<int,double> > finaldocs = getDocuments(expression,querynorm,numDocs,dictfile,indexfile);
		for (int k=0;k<min(stoi(cutoff),int(finaldocs.size()));k++){
			outfile<<filenames[get<0>(finaldocs[k])]<<" "<<get<1>(finaldocs[k])<<"\n";
		}
		outfile<<"\n\n";
	}
	outfile.close();
	return 0;
}