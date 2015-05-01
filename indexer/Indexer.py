import gzip
import os
import time
import re
import sys
import parser
import urllib2
import traceback
import cProfile

class Pageinfo:

    def __init__(self,url,num_of_tok):
        self.url=url
        self.num_of_tok=num_of_tok

class Fileprocess:

    def __init__(self):
        self.dataPath=os.getcwd()+"/data/data/4c/tux-4/polybot/gzipped_sorted_nz" # get the path of data to be process
        self.indexPath=os.getcwd()
        self.docBuf={}    
        self.indexBuf={}    
        self.docID=0
        self.avg_num_of_tok=0.0
        self.docFile=open(os.getcwd()+'/final/doc','w')
        self.indexNumber=0
        self.indexCurSize=0
        self.indexFile=open(self.indexPath+'/index.txt','w')
        self.totallen=0
        self.metaFile=open(self.indexPath+'/final/meta','w')

    def writeFile(self):
        for k,v in self.docBuf.iteritems():
            self.docFile.write(str(k)+" "+str(v.url)+" "+str(v.num_of_tok))

        for k,v in self.indexBuf.iteritems():
            self.indexFile.write(str(k)+" "+str(v)+"\n")

    def processToken(self,tok,con,pos,tokhash,tokPos):
        if tok in tokhash:
            tokhash[tok]+=1
        else:
            tokhash[tok]=1
            tokPos[tok]=[]
        tokPos[tok].append([pos,con])

    def add2IndexBuf(self,docID,tokhash,tokPos):
        for k,v in tokhash.iteritems():
            posList=tokPos[k]
            if k in self.indexBuf:
                self.indexBuf[k].extend([str(docID),str(v)])
            else:
                self.indexBuf[k]=[str(docID),str(v)]
            for pair in posList:
                self.indexBuf[k].extend([str(pair[0]),str(pair[1])])
            self.indexBuf[k][-1]+=','

    def nextIndexFile(self):
        self.indexFile.close()
        self.indexNumber+=1
        self.indexFile=open(self.indexPath+'/'+str(self.indexNumber)+'.txt','w')
        self.indexCurSize=0

#output index data in buffer to disk. Limit the size of a index file no larger than 256MB too much.
    def outIndex(self):
        '''for k,v in self.indexBuf.iteritems():
            s=str(k)+" "+str(v)+"\n"
            self.indexFile.write(s)
            self.indexCurSize+=sys.getsizeof(s)
        self.indexBuf={}'''
        '''if self.indexCurSize>268435456: #Once a index file beyond 256MB, create a new one
            self.nextIndexFile()'''
        for k,v in self.indexBuf.iteritems():
            self.indexFile.write(str(k))
            for x in v:
                self.indexFile.write(" "+x)
            self.indexFile.write("\n")
        self.indexBuf={}

#output doc table data in buffer to disk. Since we assume it could be fit in memory, I just store it in one file.
    def outDoc(self):
        for k,v in self.docBuf.iteritems():
            self.docFile.write(str(k)+" "+str(v[0])+" "+str(v[1])+"\n")
        self.docBuf={}

def valid_token(tok):
    if len(str(tok))>38:
        return False
    '''if re.search('^[0-9a-zA-Z]+$',tok.split(' ')[0]):
        return True'''
    return True
     
def main():
    htmlFile=None
    indFileIn=None
    fp=Fileprocess()
    c=1
    tokNum=0
    data_file_pattern = re.compile(r'^[0-9]+_data')
    for root, dirs, files in os.walk(fp.dataPath): 
        for f in files:
            if data_file_pattern.match(f):
                indexname=f.split('_')[0]+'_index'
                print "current file: "+os.path.join(root,f)
                
                if c%5==0:
                    fp.outIndex()
                    fp.outDoc()
                c+=1
                htmlFile = gzip.open(os.path.join(root,f),'rb')  #open data file
                indFileIn = gzip.open(os.path.join(root,indexname),'rb')  # open index file
                for line in indFileIn:
                    try:
                        i_data=line.split()
                        url=i_data[0].strip()
                        html_str=htmlFile.read(int(i_data[3])) #read size of html from index
                        if i_data[6] !='ok' and i_data[6] !='200' :
                            break
                        pool = html_str+html_str+"1" # I do not know why,according to instruction,guess related to boundary
                        p_tok_t = parser.parser(urllib2.unquote(url),html_str,pool,len(html_str)+1,len(html_str))
                        #print p_tok_t
                        p_tok = str(p_tok_t[1]).split('\n')
                        num_of_tok=0
                        fp.docID+=1
                        tokhash={}
                        tokPos={}
                        for tok in p_tok:
                            if valid_token(tok):
                                pair=tok.split(' ')
                                if len(pair)!=2:
                                    continue
                                #fp.processToken(pair[0],pair[1],num_of_tok,tokhash,tokPos)
                                tok=pair[0]
                                if tok in tokhash:
                                    tokhash[tok]+=1
                                else:
                                    tokhash[tok]=1
                                    tokPos[tok]=[]
                                tokPos[tok].append([num_of_tok,pair[1]])
                                num_of_tok+=1
                        
                        #tokNum+=num_of_tok
                        for k,v in tokhash.iteritems():
                            posList=tokPos[k]
                            if k in fp.indexBuf:
                                fp.indexBuf[k].extend([str(fp.docID),str(v)])
                            else:
                                fp.indexBuf[k]=[str(fp.docID),str(v)]
                            for pair in posList:
                                fp.indexBuf[k].extend([str(pair[0]),str(pair[1])])
                            fp.indexBuf[k][-1]+=','
                        #fp.add2IndexBuf(fp.docID,tokhash,tokPos)
                        #page=Pageinfo(url,num_of_tok)
                        fp.docBuf[fp.docID]=[url,num_of_tok]
                        fp.totallen+=num_of_tok;
                    except TypeError,e:
                        e
                        #print e
                        #i_data=line.split()
                        #url=i_data[0].strip()
                        #traceback.print_exc()
                    #finally:
                        #print urllib2.unquote(url)
                htmlFile.close()
                indFileIn.close()
    print fp.indexCurSize
    #print "token:%i"%tokNum
    #fp.writeFile()
    fp.outIndex()
    fp.outDoc()
    fp.metaFile.write(str(fp.totallen/float(fp.docID)))
    indFileIn.close()
    htmlFile.close()

if __name__ == '__main__':
    starttime=time.time()
    #fp=Fileprocess()
    main()
    #cProfile.run('fp.main')
    endtime=time.time()
    print (endtime-starttime)/60, 'minutes'

