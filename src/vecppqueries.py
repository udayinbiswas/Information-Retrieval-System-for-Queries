import nltk
from nltk.corpus import stopwords
from nltk.stem import PorterStemmer
from nltk.tokenize import word_tokenize
from os import listdir
from os.path import isfile, join
import sys

	
def apply_NLTK(inputfile,stoppingwords,stemming):
	file = open(inputfile, 'r')
	lines = file.readlines()
	stop_words = set(stopwords.words('english'))
	mod_lines = ""
	puncs = ["(",")","&","|","N",":"]
	for sentence in lines:
		words = word_tokenize(sentence)
		if int(stoppingwords)==1:
			stopword_removed_sentence = [w for w in words if not w in stop_words]
		else:
			stopword_removed_sentence = words
		mod_sentence = []

		if int(stemming)==1:
			for word in stopword_removed_sentence:
				mod_sentence.append(ps.stem(word))
		else:
			for word in stopword_removed_sentence:
				mod_sentence.append(word)
		final_sentence =""
		l = 0
		while (l<=len(mod_sentence)-2):
			if (mod_sentence[l]=="N" and mod_sentence[l+1]==":"):
				final_sentence += mod_sentence[l]+mod_sentence[l+1]+mod_sentence[l+2]+" "
				l+=3
			else :
				final_sentence += mod_sentence[l]+" "
				l+=1
		while (l<len(mod_sentence)):
			final_sentence += mod_sentence[l]+" "
			l+=1
		# print(final_sentence)
		mod_lines+=final_sentence
	file = open(inputfile,'w')
	# print(mod_lines)
	file.write(mod_lines)

# MAIN method
ps = PorterStemmer()
stop_words = set(stopwords.words('english'))
queryfile = sys.argv[1]
apply_stopwords = sys.argv[2]
apply_stemming = sys.argv[3]
apply_NLTK(queryfile,apply_stopwords,apply_stemming)
