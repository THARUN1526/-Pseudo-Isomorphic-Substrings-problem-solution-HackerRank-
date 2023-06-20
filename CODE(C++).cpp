#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <cstring>
#include <ctime>
#include <cassert>
#include <limits>
using namespace std;
typedef long long LL;
#define FOR(k,a,b) for(int k(a); k < (b); ++k)
#define FORD(k,a,b) for(int k(a); k >= (b); --k)
#define REP(k,a) for(int k=0; k < (a); ++k)
#define ABS(a) ((a)>0?(a):-(a))
 
typedef vector<int> VI;
typedef vector<vector<int> > VVI;
typedef vector<vector<bool> > VVB;
 
LL powmod(LL a, LL b,LL p)
{
        LL tmp=a;
        LL res=1;
        while(b)
        {
                if(b&1)
                {
                        res*=tmp;
                        res%=p;
                }
                tmp*=tmp;
                tmp%=p;
                b>>=1;
        }
        return res;
}
 
int main()
{
#ifdef HOME
        freopen ("in.txt","r",stdin);
        freopen ("out.txt","wb",stdout);
#endif
        char cc[100009];
        scanf("%s",cc);
        int N=strlen(cc);
        int tmp,ctr;
        vector<vector<int> > perm(26,vector<int> (N));
        vector<int> llast(26);
        map<int,int> ml;
        map<int,int>::iterator mit;
        REP(i,26)
        {
                llast[i]=1e6+i;
                ml[llast[i]]=i;
        }
        for(int i=N-1;i>-1;--i)
        {
                tmp=cc[i]-'a';
                ctr=1;
                ml.erase(llast[tmp]);
                llast[tmp]=i;
                ml[i]=tmp;
                for(mit=ml.begin();mit!=ml.end();++mit)
                {
                        perm[mit->second][i]=ctr;
                        ++ctr;
                }
        }
        vector<int> v(N,1e6);
        LL mod1=1e9+7, factor1=37;
        vector<LL> factors1(1e5+2);
        factors1[0]=1;
        LL inversefactor1=powmod(factor1,mod1-2,mod1);
        FOR(i,1,factors1.size())
                factors1[i]=(factors1[i-1]*factor1)%mod1;
        map<pair<int,LL>,int> hash;
        hash[make_pair(0,0)]=0;
        hash[make_pair(1,1)]=0;
        v[0]=1;
        int currlen=1, actsimilar=0;
        vector<LL> w1(26);
        w1[cc[1]-'a']++;
        FOR(st,1,N)
        {
                //calculate actual hash
                LL currhash=0;
                REP(i,26)
                {
                        currhash+=w1[i]*perm[i][st];
                        currhash%=mod1;
                }
                //find the first isomorph substring to the current
                while(hash.count(make_pair(currlen,currhash)))
                {
                        actsimilar=hash[make_pair(currlen,currhash)];
                        for(++currlen;st+currlen<=N;++currlen)
                        {
                                w1[cc[st+currlen-1]-'a']+=factors1[currlen-1];
                                if(w1[cc[st+currlen-1]-'a']>=mod1)
                                        w1[cc[st+currlen-1]-'a']-=mod1;        
                                currhash+=factors1[currlen-1]*perm[cc[st+currlen-1]-'a'][st];
                                currhash%=mod1;
                                if(perm[cc[actsimilar+currlen-1]-'a'][actsimilar]!=perm[cc[st+currlen-1]-'a'][st])
                                {
                                        break;
                                }
                         }
                }
                if(currlen+st>N)
                        break;
                assert(!hash.count(make_pair(currlen,currhash)));
                hash[make_pair(currlen,currhash)]=st;//insert current hash
                //insert other hash direction
                currhash+=(mod1-factors1[currlen-1])*perm[cc[st+currlen-1]-'a'][st];
                currhash%=mod1;
                currhash+=(factors1[currlen-1])*perm[cc[actsimilar+currlen-1]-'a'][actsimilar];
                currhash%=mod1;
                if(!hash.count(make_pair(currlen,currhash)))
                {
                        hash[make_pair(currlen,currhash)]=actsimilar;
                }
                v[st]=currlen;        
                //delete first character
                w1[cc[st]-'a']--;
                if(w1[cc[st]-'a']<0)
                        w1[cc[st]-'a']+=mod1;
                --currlen;
                if(currlen>1)
                {
                        w1[cc[st+currlen]-'a']-=factors1[currlen];
                        if(w1[cc[st+currlen]-'a']<0)
                                w1[cc[st+currlen]-'a']+=mod1;
                        --currlen;
                }
                currhash=0;
                REP(i,26)
                {
                        w1[i]*=inversefactor1;
                        w1[i]%=mod1;
                        currhash+=w1[i]*perm[i][st+1];
                        currhash%=mod1;
                }
                if(currlen>1 && !hash.count(make_pair(currlen,currhash)))
                {
                        hash[make_pair(currlen,currhash)]=actsimilar+1;
                }          
        }
        vector<LL> vz(N),vv(N);
        REP(i,N)
        {
                if(i+v[i]<=N)
                        vz[i+v[i]-1]++;
        }
        vv[0]=vz[0];
        FOR(i,1,N)
        {
            vz[i]+=vz[i-1];
            vv[i]=vv[i-1]+vz[i];    
        }
        REP(i,N)
            printf("%lld\n",vv[i]);
        return 0;
}