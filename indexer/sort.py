import subprocess
import sys
import time
import os
import re

class Sort:

	def __init__(self):
		self.indexFile=None
		self.lexiconFile=None
		self.inBuf=[]
		self.outBuf=[]

	def UnixSort(self):
		print os.getcwd()
		#subprocess.call(['sort -k 1 -o '+os.getcwd()+'/index/tgt.txt '+os.getcwd()+'/index/index.txt'])
		subprocess.call(['sort','-k','1','-o','tgt.txt','index.txt'])

	def mergeDup(self):
		self.lexiconFile=open(os.getcwd()+"/final/lexicon",'w')
		self.indexFile=open(os.getcwd()+"/final/index",'w')
		bufSize=0
		i=0
		with open(os.getcwd()+"/tgt.txt") as index:
			dupList=[]
			prev=None
			dupFlag=False
			for line in index:
				#print sys.getsizeof(self.outBuf)
				record=line.split(' ',1)
				if record[0]==prev:
					dupFlag=True
					a=re.split(r',\s*',record[1])[:-1]
					dupList.extend(a)
				else:
					if dupFlag:
						dupList=sorted(dupList, key=lambda p: p.split(' ',1)[0])
						dupFlag=False
					#print str(dupList)
					if prev:
						self.outBuf.append([prev,dupList])
					prev=record[0]
					dupList=[]
					a=re.split(r',\s*',record[1])[:-1]
					dupList.extend(a)
					if sys.getsizeof(self.outBuf)>52428800: #output buffer once beyond 50MB
						for record in self.outBuf:
							self.lexiconFile.write(record[0]+' '+str(i)+'\n')
							self.indexFile.write(record[0]+' ')
							i+=1
							for a in record[1]:
								self.indexFile.write(a+',')
							self.indexFile.write('\n')
		print sys.getsizeof(self.outBuf)
		for record in self.outBuf:
			self.lexiconFile.write(record[0]+' '+str(i)+'\n')
			self.indexFile.write(record[0]+' ')
			i+=1
			for a in record[1]:
				self.indexFile.write(a+',')
			self.indexFile.write('\n')
		self.indexFile.close()

if __name__ == '__main__':
    starttime=time.time()
    s=Sort()
    s.UnixSort()
    print "********************************************"
    s.mergeDup()
    endtime=time.time()
    print (endtime-starttime)/60, 'minutes'
