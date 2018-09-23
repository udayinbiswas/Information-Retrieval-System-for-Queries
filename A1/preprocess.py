import nltk
from nltk.corpus import stopwords
from nltk.stem import PorterStemmer
from nltk.tokenize import word_tokenize
from os import listdir
from os.path import isfile, join
import sys

	
def apply_NLTK(inputfile,stoppingwords,stemming,collectionpath):
	if collectionpath[-1]=='/':
		file= open(collectionpath+inputfile, 'r')
	else:
		file = open(collectionpath+'/'+inputfile, 'r')
	lines = file.read().split("\n")
	mod_lines = ""
	for sentence in lines:
		if sentence!='\n' and sentence!='':
			stop_words = set(stopwords.words('english'))
			words = word_tokenize(sentence)
			if int(stoppingwords)==1:
				stopword_removed_sentence = [w for w in words if not w in stop_words]
			else:
				stopword_removed_sentence = words
			mod_sentence = ""
			if int(stemming)==1:
				for word in stopword_removed_sentence:
					mod_sentence+= ps.stem(word)+" "
			else:
				for word in stopword_removed_sentence:
					mod_sentence+= word +" "
			mod_lines+=mod_sentence+'\n'
	file = open('preprocessed/pp_'+inputfile,'w')
	file.write(mod_lines)


# MAIN method
ps = PorterStemmer()
stop_words = set(stopwords.words('english'))
print("Preprocessing...")
collectionpath = sys.argv[1]
apply_stopwords = sys.argv[2]
apply_stemming = sys.argv[3]
onlyfiles = [f for f in listdir(collectionpath) if isfile(join(collectionpath, f))]
x = open("pp_files.txt",'w')
for f in onlyfiles:
	x.write('pp_'+f+'\n')
x = open("original_files.txt",'w')
for f in onlyfiles:
	x.write(f+'\n')
for f in onlyfiles:
	apply_NLTK(f,apply_stopwords,apply_stemming,collectionpath)

print("Done Preprocessing.")