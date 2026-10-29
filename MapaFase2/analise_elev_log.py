import re,bisect,sys
d=sys.argv[1]
def parse(f):
    S={}
    for l in open(f):
        m=re.match(r'srvT=([\d.]+) ph=(\d+) elevZ=([\-\d.]+) mov=(\d) portasLat=([\d.]+) portao=([\d.]+)',l)
        if m: S[float(m[1])]=(float(m[3]),int(m[4]),float(m[5]),float(m[6]),l)
    return S
for tag in ('host','client'):
    S=parse(f'{d}/log_{tag}.txt')
    bad=[(t,v[0],v[2],v[3]) for t,v in S.items() if 2<v[0]<598 and (v[2]>0.01 or v[3]>0.01)]
    rel=[int(r) for t,v in S.items() if 2<v[0]<598 for r in re.findall(r'rel=\(-?\d+,-?\d+,(-?\d+)\) base=CoopElevator',v[4])]
    nb=sum(1 for t,v in S.items() if 2<v[0]<598 for b in re.findall(r'base=(\w+)\]',v[4]) if not b.startswith('CoopElevator'))
    print(tag,'amostras',len(S),'| em movimento c/ porta aberta:',len(bad),bad[:2],'| relZ mov',min(rel),max(rel),'n',len(rel),'| base!=elev em mov:',nb)
h=parse(f'{d}/log_host.txt'); c=parse(f'{d}/log_client.txt'); ts=sorted(h)
for flag in (1,0):
    e=[]
    for t,v in c.items():
        if v[1]!=flag: continue
        i=bisect.bisect_left(ts,t)
        if 0<i<len(ts) and ts[i]-ts[i-1]<0.6:
            a,b=h[ts[i-1]][0],h[ts[i]][0]; zh=a+(b-a)*(t-ts[i-1])/(ts[i]-ts[i-1]); e.append(abs(zh-v[0]))
    print('sync elevZ', 'movendo' if flag else 'parado', len(e), 'max %.2f media %.2f'%(max(e),sum(e)/len(e)))
