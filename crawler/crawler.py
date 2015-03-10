#encoding=utf-8
import socket
import urllib2, urllib
import robotparser
import re
import zlib
import htmllib,formatter,string
from pygoogle import pygoogle
import sys

class GetContent(htmllib.HTMLParser):
    
    def __init__(self):
        self.links={}
        f = formatter.NullFormatter()
        htmllib.HTMLParser.__init__(self,f)
        self.body=False
        self.gate=True
        self.content=""

    def handle_starttag(self, tag, method, attrs):
        if tag=='body':
            self.body=True
        elif tag=='script':
            self.gate=False

    def handle_endtag(self, tag, method):
        if tag=='body':
            self.body=False
        elif tag=='script':
            self.gate=True

    def handle_data(self, data):
        if self.body and self.gate and not data is None:
            self.content+=data+" "
    
class GetLinks(htmllib.HTMLParser):
    def __init__(self):
        self.links={}
        f = formatter.NullFormatter()
        htmllib.HTMLParser.__init__(self,f)
        self.body=False
        self.gate=True
    
    def anchor_bgn(self,href,name,type):
        self.save_bgn()
        self.link=href
            
    def anchor_end(self):
        saveend=self.save_end()
        if not saveend is None:
            text=string.strip(saveend)
            if self.link and text:
                self.links[text]=self.link

class MyCrawler:
    def __init__(self,seeds,kw):
        self.current_depth = 1
        self.linkQueue=linkQueue()
        self.kw=kw.split(" ")#********
        self.totalSize=0
        self.totalDownload=0
        if isinstance(seeds,str):
            self.linkQueue.addUnvisitedUrl(seeds)
        if isinstance(seeds,list):
            for i in seeds:
                self.linkQueue.addUnvisitedUrl(i)

        #print "Add the seeds url \"%s\" to the unvisited url list"%str(self.linkQueue.unVisited)

    def crawling(self,number):
        while self.totalDownload<number:
            print "***********level %i begin**********"%self.current_depth
            #bypass a layer
            links=[]#links for next layer
            while not self.linkQueue.unVisitedUrlsEmpty() and self.totalDownload<number:
                visitUrl=self.linkQueue.unVisitedUrlDeQueue()
                print "-------------------------------"
                print "Visiting \"%s\""%visitUrl
                #get base url and read robots.txt
                try:
                    base = re.search('(http[s]?://(?:[a-zA-Z]|[0-9]|[$-_@.&+])+?)(?:/|\s*$)', visitUrl)
                    baseurl=base.group(1)
                    rp = robotparser.RobotFileParser()
                    rp.set_url(baseurl+"/robots.txt")
                    rp.read()
                    if not rp.can_fetch("*",visitUrl):
                        print "Robot access prohibited"
                        #mark visited url
                        self.linkQueue.addVisitedUrl(visitUrl)
                        continue
                except:
                    print "Invalid robots.txt url"
                    #mark visited url
                    self.linkQueue.addVisitedUrl(visitUrl)
                    continue
                if visitUrl is None or visitUrl=="":
                    print "Empty url"
                    continue
                pagesrc=self.getPageSrc(visitUrl)
                if pagesrc is None:
                    print "Unable to get page source: url may be invalid"
                    #mark visited url
                    self.linkQueue.addVisitedUrl(visitUrl)
                    continue
                #parse page
                if pagesrc[0] == 0:
                    parsedData=self.getParsedData(pagesrc[1])
                    if parsedData is None:
                        print "Unable to fetch page source"
                        #mark visited url
                        self.linkQueue.addVisitedUrl(visitUrl)
                        continue
                    score=self.totalCount(parsedData[0])
                    print "score:"+str(score)
                    
                    curLinks=[{'score':score,'link':x} for x in parsedData[1]]
                    links.extend(curLinks)

                #store file
                fn=re.sub(r'^.*?://','',visitUrl)
                fn=re.sub(r'\n$','',fn)
                fn=re.sub(r'[\/:*?"<>|]','_',fn)
                if pagesrc[0]==1:
                    urllib.urlretrieve(visitUrl,'pages/'+fn)
                else:
                    result=open('pages/'+fn+'.'+pagesrc[2],'w')
                    result.write(pagesrc[1])
                    result.close()

                self.totalDownload+=1
                self.totalSize+=pagesrc[3]
                
                print "Get %d new links"%len(links)
                #mark visited url
                self.linkQueue.addVisitedUrl(visitUrl)
                print "Visited url count: "+str(self.linkQueue.getVisitedUrlCount())
                #print "Visited depth: "+str(self.current_depth)
            #the following steps generating priority links queue without duplicate
            #sort links by url
            sortedLinks=sorted(links,key=lambda i:i['link'])
            #eliminate duplicate links and average score
            prevLink=sortedLinks[0]['link']
            dupNum=1
            acumScore=sortedLinks[0]['score']
            adjustLinks=[]
            for i in range(1,len(sortedLinks)):
                curLink=sortedLinks[i]
                if prevLink==curLink['link']:
                    dupNum+=1
                    acumScore+=curLink['score']
                else:
                    adjustLinks.append({'score':float(acumScore/dupNum),'link':prevLink})
                    dupNum=1
                    acumScore=curLink['score']
                prevLink=curLink['link']
            adjustLinks.append({'score':float(acumScore/dupNum),'link':prevLink})
            #sort links by score
            sortedLinks=sorted(adjustLinks,key=lambda i:i['score'],reverse=True)
            for link in sortedLinks:
                self.linkQueue.addUnvisitedUrl(link['link'])
            print "%d unvisited links:"%len(self.linkQueue.getUnvisitedUrl())
            self.current_depth += 1
            if self.linkQueue.unVisitedUrlsEmpty():
                print "crawling fail: dead end"
                break
        print "*********************************"
        print "total download: %d"%self.totalDownload
        self.totalSize=float(self.totalSize)/(1024*1024)
        print "total size: %fMB"%self.totalSize
  
    #return a list with page content and links
    def getParsedData(self,src):
        links=[]
        content=""
        getLinks=GetLinks()
        getContent=GetContent()
        try:
            getLinks.feed(src)
            getLinks.close()
            for href,link in getLinks.links.iteritems():
                if link.find("http://")!=-1:
                    #print link
                    links.append(link)
            #print 'links:  ' +str(links[0:3])
            links.extend(re.findall('http[s]?://(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\(\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))+', src))
            links=set(links)
            links=list(links)
            getContent.feed(src)
            getContent.close()
            content=getContent.content
            return [content,links]
        except:
            #getLinks.close()
            getContent.close()
            return None

    #this will return a file type with source, interested type include:
    #0:text/html
    #1:image
    #2:audio
    #3:video
    def getPageSrc(self,url,timeout=100,coding=None):
        try:
            socket.setdefaulttimeout(timeout)
            req = urllib2.Request(url)
            req.add_header('User-agent', 'Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)')
            response = urllib2.urlopen(req)
            page = '' 
            ptype = -1
            extend=""
            htmlpt=re.compile(r'^text/html')
            imagept=re.compile(r'^image/([\w\d]*)')
            audiopt=re.compile(r'^audio/([\w\d]*)')
            videopt=re.compile(r'^video/([\w\d]*)')
            doctype=response.headers.get('Content-Type')
            size=int(response.headers.get('Content-Length'))
            if htmlpt.match(doctype):
                #print response.headers.get('Content-Type')
                ptype=0
                extend="html"
            elif imagept.match(doctype):
                ptype=1
                m=imagept.match(doctype)
                extend=m.group(1)
            elif audiopt.match(doctype):
                ptype=2
                m=audiopt.match(doctype)
                extend=m.group(1)
            elif videopt.match(doctype):
                ptype=3
                m=videopt.match(doctype)
                extend=m.group(1)
            else:
                return None
            if response.headers.get('Content-Encoding') == 'gzip': 
                page = zlib.decompress(page, 16+zlib.MAX_WBITS)
            if coding is None:   
                coding= response.headers.getparam("charset")   
            if coding is None:   
                page=response.read()   
            else:           
                page=response.read()   
                page=page.decode(coding).encode('utf-8')
            if size==None:
                size=len(page)
            return [ptype,page,extend,size]
        except Exception,e:
            print str(e)
            return None

    def totalCount(self, content):
        kwlist=[x.lower() for x in self.kw]
        kwset=set(kwlist)
        #print kwset
        content=re.sub(r'\s+',' ',content)
        tokens=content.split(" ")
        count=0
        #print "tokens:"
        for token in tokens:
            token=token.lower()
            #print token
            if token in kwset:
                count+=1
        return count
     
class linkQueue:
    def __init__(self):
     #set of visited urls
        self.visted=[]
     #set of urls to be visited
        self.unVisited=[]
 #get list of visited urls
    def getVisitedUrl(self):
        return self.visted
 #get list of unvisited urls
    def getUnvisitedUrl(self):
        return self.unVisited
 #add an url to visited list
    def addVisitedUrl(self,url):
        self.visted.append(url)

    def removeVisitedUrl(self,url):
        self.visted.remove(url)
 #get next url to be visited from unvisited list
    def unVisitedUrlDeQueue(self):
        try:
            return self.unVisited.pop()
        except:
            return None
 #add an url to unvisited list
    def addUnvisitedUrl(self,url):
        if url!="" and url not in self.visted and url not in self.unVisited:#check if this url is visited
            self.unVisited.insert(0,url)

    def getVisitedUrlCount(self):
        return len(self.visted)

    def getUnvistedUrlCount(self):
        return len(self.unVisited)

    def unVisitedUrlsEmpty(self):
        return len(self.unVisited)==0
 
def run(seeds,number,kw):
    craw=MyCrawler(seeds,kw)
    craw.crawling(number)
#get 10 results from google
def queryGoogle(kw):
    g = pygoogle(kw)
    g.pages = 3
    result=g.get_urls()
    for r in result:
        print r
    return result[0:10]

def query(kw,number):
    gResult=queryGoogle(kw)
    run(gResult,number,kw)
 
if __name__=="__main__":
    kw=sys.argv[1]
    kw=re.sub('_',' ',kw)
    number=sys.argv[2]
    #kw='billy harrington'
    #number=30
    gResult=queryGoogle(kw)
    run(gResult,number,kw)