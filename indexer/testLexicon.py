import os
lexiconFile=open(os.getcwd()+"/final/lexicon",'r')
for c in xrange(7):
	with open(os.getcwd()+"/final/index"+str(c)) as index:
		i=0
		for line in index:
			record=lexiconFile.readline()
			record=record.split()
			ilist=line.split(' ',1)
			#print ilist[0]
			#print record[0]
			if ilist[0] != record[0]:
				print "error1"
				print i,c,record[0]
				print record[2]
				break
			elif int(record[1])!=int(c):
				print "error2"
				print i,c,record[1]
				print record[2]
				break
			elif int(record[2])!=int(i):
				print "error3"
				print i,c,record[2]
				print record[1]
				break
			i+=1
	print i
lexiconFile.close() 
