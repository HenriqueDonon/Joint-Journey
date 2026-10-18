# Gerador do painel. Para atualizar: edite CONCLUIDAS / ATUALIZADO_EM abaixo e rode:  python gerar_painel.py
import html, os
S = os.path.dirname(os.path.abspath(__file__)) + os.sep
bg = open(S + 'fundo.b64.txt').read().strip()

# ==== CONTROLE DE ETAPAS (atualizado por Claude quando a equipe conclui uma etapa) ====
CONCLUIDAS = []            # ex.: ['01', '04']  -> aparecem como "Concluída"
ATUALIZADO_EM = ''         # ex.: '2026-10-18'
e = html.escape

# status: c=confirmado, v=nao verificado / a verificar, p=planejado/pendente/futuro, d=em desenvolvimento
ST = {'k': 'Concluída', 'c': 'Confirmado', 'v': 'Não verificado', 'p': 'Planejado', 'd': 'Em desenvolvimento'}

prior = [
 ('01','Backup e GitHub','Configurado, recuperação a verificar','v'),
 ('02','Multiplayer para dois','Testado pela equipe','c'),
 ('03','Interações básicas','Funcional segundo a equipe','c'),
 ('04','Poderes de Ruína e Neon','Estado atual a confirmar','v'),
 ('05','Blockout da primeira fase','Estado atual a confirmar','v'),
 ('06','Puzzle da coluna e ponte','Planejado','p'),
 ('07','Colapso e pressão de tempo','Planejado','p'),
 ('08','Validação integrada da primeira fase','Pendente','p'),
 ('09','Menu e progressão','Pendente','p'),
 ('10','Acabamento visual da primeira fase','Pendente','p'),
 ('11','Novos mapas e mecânicas','Futuro','p'),
 ('12','Inimigos e furtividade','Futuro','p'),
 ('13','História e cutscenes','Planejamento paralelo','p'),
 ('14','Otimização e testes finais','Futuro','p'),
 ('15','Missões extras para três jogadores','Expansão opcional','p'),
]

# etapas: (num, titulo, status, [blocos]) ; bloco = (tipo, titulo, conteudo)  tipo: p=paragrafo, l=lista, o=lista numerada, n=nota
etapas = [
('01','BACKUP E CONTROLE DE VERSÃO','v',[
 ('p','Objetivo','Proteger o projeto contra perdas e alterações incorretas.'),
 ('l','Entregas',['Repositório Git organizado.','Arquivos essenciais versionados.','Histórico de modificações.','Possibilidade de reverter alterações.','Backup independente quando necessário.','Procedimento conhecido de restauração.']),
 ('p','Critério de conclusão','Demonstrar que uma versão anterior pode ser recuperada sem perda dos recursos essenciais.'),
 ('n','','GitHub não deve ser tratado como substituto de todo e qualquer backup.')]),
('02','MULTIPLAYER PARA DOIS JOGADORES','c',[
 ('p','Objetivo','Garantir a experiência cooperativa entre Ruína e Neon em computadores diferentes.'),
 ('l','Entregas',['Conexão entre host e convidado.','Movimentação sincronizada.','Interações replicadas corretamente.','Estados dos objetos consistentes.','Testes de estabilidade.']),
 ('p','Critério de conclusão','Ambos conseguem entrar na partida, movimentar-se e interagir corretamente.'),
 ('p','Situação','A equipe já confirmou funcionamento satisfatório nos dois computadores.'),
 ('n','','Novas mecânicas precisarão de testes de rede próprios.')]),
('03','SISTEMAS BÁSICOS DE INTERAÇÃO','c',[
 ('p','Objetivo','Manter uma base reutilizável para puzzles.'),
 ('l','Entregas',['Botões.','Portas.','Caixas.','Sensores.','Objetivos de sala.','Interações entre jogadores e objetos.']),
 ('p','Critério de conclusão','Os sistemas respondem corretamente às interações e mantêm consistência entre os computadores.'),
 ('n','Observação','Essas funcionalidades já foram relatadas como operacionais. Reutilizar o que existe, sem recriar desnecessariamente.')]),
('04','PODERES DEFINITIVOS','v',[
 ('p','Objetivo','Implementar a identidade principal de Ruína e Neon.'),
 ('l','Ruína',['Destruir objetos preparados para a habilidade.','Alterar estados de estruturas.','Disponibilizar materiais reutilizáveis.']),
 ('l','Neon',['Reconstruir objetos.','Utilizar materiais disponíveis.','Construir estruturas em locais definidos.','Restaurar mecanismos.']),
 ('p','Primeiro protótipo','Ruína destrói um objeto e Neon o reconstrói.'),
 ('p','Segunda evolução','Os materiais provenientes de um objeto destruído podem construir outro objeto.'),
 ('p','Cuidados','Replicação multiplayer, autoridade do servidor, colisões, limites de interação e organização modular.'),
 ('p','Critério de conclusão','As habilidades funcionam repetidamente nos dois computadores, sem erros críticos.'),
 ('n','','Não remover os poderes provisórios antes de validar a substituição.')]),
('05','BLOCKOUT DA PRIMEIRA FASE','v',[
 ('p','Objetivo','Criar a estrutura espacial da primeira fase antes de utilizar modelos detalhados.'),
 ('l','Elementos',['Plataforma inicial.','Coluna rachada.','Vão perigoso.','Ponte danificada.','Plataforma de chegada.','Saída.','Teto e paredes provisórios.']),
 ('l','Entregas',['Mapa com geometria simples.','Escalas e alturas definidas.','Colisões corretas.','Distâncias entre estruturas registradas.','Viabilidade do alcance dos poderes analisada.','Preparação para substituição futura por modelos do Blender.']),
 ('p','Critério de conclusão','Os jogadores conseguem explorar a sala, e uma ponte provisória habilitada manualmente permite atravessar o vão.'),
 ('n','','Não exigir acabamento gráfico nesta etapa.')]),
('06','PRIMEIRO PUZZLE COMPLETO','p',[
 ('p','Nome provisório','A Ponte do Colapso.'),
 ('p','Objetivo','Validar a primeira situação de cooperação verdadeira.'),
 ('o','Sequência',['Ruína e Neon chegam ao laboratório.','Encontram uma ponte destruída e uma coluna rachada sustentando o teto.','Ruína destrói a coluna.','Os materiais da coluna ficam disponíveis.','Neon utiliza os materiais para construir a ponte.','Os dois atravessam.','Ruína destrói a ponte após a travessia.','Neon recupera os materiais e reconstrói a coluna original.','A sustentação é restaurada.','Os dois podem prosseguir.']),
 ('p','Regra fundamental','A coluna e a ponte compartilham o mesmo conjunto de materiais.'),
 ('l','Cuidados',['Alcance dos poderes.','Linha de visão.','Estados exclusivos das estruturas.','Transferência compreensível dos materiais.','Sincronização.','Prevenção de situações sem solução.']),
 ('p','Critério de conclusão','A dupla executa toda a sequência sem intervenção manual.'),
 ('n','','Não exigir Vértice.')]),
('07','COLAPSO E PRESSÃO DE TEMPO','p',[
 ('p','Objetivo','Transformar o primeiro puzzle em uma situação de tensão cooperativa.'),
 ('l','Proposta',['Ruína destrói a coluna.','O teto começa a rachar.','Surgem alarmes, ruídos e sinais de instabilidade.','Neon precisa reconstruir a ponte.','Ambos atravessam.','Ruína destrói a ponte.','Neon reconstrói a coluna antes do colapso.']),
 ('p','Direção de design','A primeira fase deve ensinar o básico antes de exigir execução rápida. O desafio cronometrado deve ocorrer após os jogadores compreenderem o funcionamento dos poderes.'),
 ('l','Cuidados técnicos',['Eventos controlados.','Temporizador replicado.','Efeitos leves.','Condição de falha.','Reinício confiável.','Tempo ajustado por testes reais.']),
 ('p','Critério de conclusão','A fase gera tensão sem se tornar injusta ou confusa.'),
 ('n','','Evitar simulação física complexa de todo o teto na primeira implementação.')]),
('08','VALIDAÇÃO INTEGRADA','p',[
 ('p','Objetivo','Garantir que a primeira fase funciona por completo.'),
 ('l','Entregas',['Testes com host e convidado.','Verificação dos poderes.','Verificação da ponte.','Verificação das colisões.','Verificação de estados sincronizados.','Testes de falha e reinício.','Testes de desconexão conforme os recursos disponíveis.','Registro de erros e correções.']),
 ('p','Critério de conclusão','Dois jogadores completam a fase sem intervenções técnicas e sem falhas críticas identificadas.'),
 ('n','','Esta etapa complementa os testes contínuos, não os substitui.')]),
('09','MENU, CHECKPOINTS E PROGRESSÃO','p',[
 ('p','Objetivo','Permitir que a dupla inicie e avance na campanha.'),
 ('l','Entregas',['Menu inicial básico.','Criar sessão.','Entrar na sessão.','Iniciar campanha.','Reiniciar fase.','Concluir objetivo.','Carregar próxima fase.','Checkpoints quando necessários.','Configurações essenciais.']),
 ('n','','A identidade visual do menu já possui referências desenvolvidas pela equipe.'),
 ('p','Critério de conclusão','Os jogadores conseguem iniciar e concluir uma fase sem comandos técnicos do editor.')]),
('10','ACABAMENTO VISUAL DA PRIMEIRA FASE','p',[
 ('p','Objetivo','Estabelecer a identidade artística do laboratório.'),
 ('l','Entregas',['Importação dos modelos 3D aprovados.','Substituição dos blocos provisórios.','Coluna e ponte detalhadas.','Estruturas industriais.','Iluminação.','Fogo e efeitos ambientais.','Feedback dos poderes.','Materiais e texturas.','Otimização inicial da cena.']),
 ('p','Critério de conclusão','A primeira fase apresenta visual consistente, mantém as interações funcionando e possui desempenho satisfatório nos computadores-alvo.'),
 ('n','','Esta é uma primeira referência artística. O polimento definitivo continua nas etapas posteriores.')]),
('11','NOVOS MAPAS E MECÂNICAS','p',[
 ('p','Objetivo','Expandir o jogo com desafios progressivamente mais elaborados.'),
 ('p','Meta criativa','Desenvolver uma campanha potencialmente com cerca de 30 mapas completos, sem comprometer a qualidade para atingir um número.'),
 ('l','Possibilidades',['Contrapesos.','Cargas que caem.','Reações em cadeia.','Esteiras industriais.','Física de objetos.','Máquinas e circuitos.','Comunicação assimétrica.','Desafios musicais.','Salas interligadas.','Gravidade zero.','Gravidade invertida.','Propulsão por impulso.','Alteração temporária de tamanho.','Múltiplas soluções.','Reaproveitamento de materiais.']),
 ('l','Progressão sugerida',['Primeira fase: introdução.','Início da campanha: normalmente 2 a 3 puzzles por mapa.','Meio da campanha: normalmente 3 a 4 puzzles.','Fases avançadas: sequências maiores e mais interligadas.']),
 ('n','','Esses números são referências, não regras obrigatórias.'),
 ('p','Critério de conclusão','Cada mapa oferece cooperação, desafios variados, progressão clara e testes adequados.'),
 ('n','','Não implementar todas as mecânicas simultaneamente.')]),
('12','INIMIGOS E FURTIVIDADE','p',[
 ('p','Objetivo','Introduzir variedade sem transformar o jogo em um FPS convencional.'),
 ('l','Possibilidades',['Robôs patrulheiros.','Câmeras de vigilância.','Sensores.','Alarmes.','Ruídos que atraem inimigos.','Coberturas.','Distrações.','Neutralização temporária de ameaças.','Rotas alternativas.']),
 ('p','Primeiro protótipo','Um inimigo, um sensor e um pequeno desafio cooperativo.'),
 ('p','Critério de conclusão','O stealth exige colaboração e funciona de maneira consistente no multiplayer.'),
 ('n','','A arma silenciosa futurista continua sendo uma possibilidade, não uma mecânica aprovada.')]),
('13','HISTÓRIA, DIÁLOGOS E CUTSCENES','p',[
 ('p','Objetivo','Construir a identidade narrativa do jogo.'),
 ('l','Direção inicial',['Ficção científica e mistério.','Ruína e Neon possuem personalidades conflitantes.','A cooperação transforma a relação entre eles.','Vértice participa da história.','Humor discreto e easter eggs.']),
 ('l','Estratégia',['Planejar a narrativa em paralelo ao desenvolvimento dos primeiros mapas.','Definir motivações e acontecimentos principais.','Organizar os momentos narrativos.','Escrever diálogos.','Implementar cutscenes somente quando as cenas e mapas relacionados estiverem suficientemente definidos.']),
 ('p','Critério de conclusão','A história complementa o gameplay e possui continuidade coerente.'),
 ('n','','Não construir cenas cinematográficas definitivas prematuramente.')]),
('14','OTIMIZAÇÃO E TESTES FINAIS','p',[
 ('p','Objetivo','Preparar o jogo para uma experiência estável.'),
 ('l','Entregas',['Análise de FPS.','Otimização de malhas.','Otimização de materiais.','Avaliação de memória.','Testes de iluminação e efeitos.','Correção de colisões.','Testes de rede.','Testes de carregamento.','Revisão dos estados dos puzzles.','Correção de falhas.','Testes prolongados da campanha.']),
 ('p','Meta inicial','Buscar fluidez, utilizando 60 FPS como referência a validar nos computadores-alvo.'),
 ('p','Critério de conclusão','Não haver problemas críticos conhecidos e o desempenho estar dentro das metas aprovadas.'),
 ('n','','A otimização deve ser acompanhada desde o início, mesmo existindo uma etapa final específica.')]),
('15','MISSÕES EXTRAS PARA TRÊS JOGADORES','p',[
 ('p','Objetivo','Permitir experiências adicionais com Ruína, Neon e Vértice.'),
 ('l','Conceito',['Campanha principal continua sendo Duo.','Missões para três jogadores poderão ser desenvolvidas futuramente.','Vértice terá papel próprio.','Suas habilidades deverão ser necessárias para resolver os desafios.']),
 ('l','Possibilidades de Vértice',['Transformação de materiais.','Alteração de propriedades.','Modificação de condições ambientais.']),
 ('n','','As habilidades específicas ainda dependem de decisões de game design.'),
 ('p','Critério de conclusão','Três jogadores conseguem participar de missões onde todos são realmente indispensáveis.'),
 ('n','','Esta expansão não deve bloquear o desenvolvimento da campanha principal.')]),
]

prior = [(n, t, 'Concluída' if n in CONCLUIDAS else s, 'k' if n in CONCLUIDAS else k) for n, t, s, k in prior]
etapas = [(n, t, 'k' if n in CONCLUIDAS else k, b) for n, t, k, b in etapas]
NC = len(CONCLUIDAS)

marcos = [
 ('A','Base cooperativa',['Multiplayer funcional.','Interações básicas funcionando.','Poderes de Ruína e Neon validados.','Primeiro objeto destrutível e reconstruível testado.']),
 ('B','Primeira fase jogável',['Coluna destrutível.','Ponte construída com material reaproveitado.','Travessia cooperativa.','Destruição da ponte.','Reconstrução da coluna.','Condição de colapso e reinício.','Conclusão da fase.']),
 ('C','Primeira fase com identidade visual',['Modelos 3D integrados.','Materiais e iluminação aprovados.','Efeitos dos poderes funcionando.','Desempenho verificado.']),
 ('D','Campanha expandida',['Múltiplos mapas.','Puzzles variados.','Progressão entre fases.','Desafios mais elaborados.','Experiência cooperativa consistente.']),
 ('E','Experiência completa',['Narrativa integrada.','Menus.','Progressão.','Conteúdo planejado validado.','Otimização.','Testes de estabilidade.','Preparação para testes externos.']),
]

metodo = ['Definir uma tarefa específica.','Inspecionar o estado real dos arquivos relevantes.','Verificar backup e dependências.','Executar apenas o trabalho autorizado.','Realizar testes adequados.','Registrar arquivos modificados e resultados.','Analisar o relatório técnico.','Aprovar ou corrigir antes de avançar.']
regras = ['Uma tarefa de execução por vez, por padrão.','Não implementar funcionalidades futuras sem autorização.','Não recriar sistemas que já funcionam.','Não modificar recursos fora do escopo.','Não declarar testes que não ocorreram.','Preservar modelos e decisões visuais aprovadas.','Manter formas de reversão.','Priorizar soluções simples e reutilizáveis.','Evitar refatorações desnecessárias.','Priorizar estabilidade multiplayer.','Utilizar o Claude de forma eficiente, reduzindo investigações repetidas e retrabalho.']

def ul(items, tag='ul'):
    return '<%s>%s</%s>' % (tag, ''.join('<li>%s</li>' % e(i) for i in items), tag)

def badge(k, text=None):
    return '<span class="badge b-%s"><i></i>%s</span>' % (k, e(text or ST[k]))

def block(b):
    t, ti, c = b
    if t == 'n':
        return '<p class="note">%s%s</p>' % ('<strong>%s:</strong> ' % e(ti) if ti else '', e(c))
    h = '<h4>%s</h4>' % e(ti)
    if t == 'p': return '<div class="blk">%s<p>%s</p></div>' % (h, e(c))
    if t == 'l': return '<div class="blk">%s%s</div>' % (h, ul(c))
    if t == 'o': return '<div class="blk">%s%s</div>' % (h, ul(c, 'ol'))

rows = ''.join('<tr><td class="n"><a href="#etapa-%s">%s</a></td><td><a href="#etapa-%s">%s</a></td><td>%s</td></tr>' % (n, n, n, e(t), badge(k, s)) for n, t, s, k in prior)
idx = ''.join('<a href="#etapa-%s"><b>%s</b>%s</a>' % (n, n, e(t.split(' | ')[0] if False else t)) for n, t, s, k in prior)
etp = ''
for n, t, k, bl in etapas:
    etp += '<article class="card etapa" id="etapa-%s"><header><span class="num">%s</span><h3>ETAPA %s <em>|</em> %s</h3>%s</header><div class="body">%s</div></article>' % (n, n, n, e(t), badge(k), ''.join(block(b) for b in bl))
mc = ''.join('<div class="card marco"><h3><span class="mk">%s</span>MARCO %s <em>|</em> %s</h3>%s</div>' % (l, l, e(t), ul(i)) for l, t, i in marcos)

CSS = """
:root{--bg:#07090f;--card:rgba(10,15,27,.86);--line:rgba(120,150,200,.22);--tx:#e8edf6;--mu:#a9b6cb;--cy:#27e1ff;--pu:#b46bff;--am:#ffb02e;--ok:#4ade80;--wa:#ff7a59;--pl:#8fa2bd;--bl:#5aa9ff}
*{box-sizing:border-box}html{scroll-behavior:smooth;scroll-padding-top:70px}
body{margin:0;background:var(--bg);color:var(--tx);font:16px/1.6 "Segoe UI",system-ui,-apple-system,Roboto,sans-serif}
body::before{content:"";position:fixed;inset:0;z-index:-2;background:url(data:image/jpeg;base64,%BG%) center/cover no-repeat}
body::after{content:"";position:fixed;inset:0;z-index:-1;background:linear-gradient(180deg,rgba(5,8,15,.55),rgba(5,8,15,.82) 40%,rgba(5,8,15,.9))}
a{color:var(--cy);text-decoration:none}a:hover{text-decoration:underline}
nav.top{position:sticky;top:0;z-index:10;background:rgba(5,8,15,.92);backdrop-filter:blur(8px);border-bottom:1px solid var(--line);display:flex;gap:4px;overflow-x:auto;padding:8px 12px;white-space:nowrap}
nav.top a{color:var(--mu);padding:6px 12px;border-radius:8px;font-size:14px;font-weight:600}nav.top a:hover{background:rgba(39,225,255,.12);color:var(--cy);text-decoration:none}
.wrap{max-width:1100px;margin:0 auto;padding:0 16px 60px}
.hero{padding:72px 0 40px;text-align:center;text-shadow:0 2px 18px #000}
.hero .kick{letter-spacing:.35em;font-size:13px;color:var(--cy);font-weight:700}
.hero h1{font-size:clamp(42px,9vw,92px);letter-spacing:.08em;margin:8px 0 0;line-height:1;background:linear-gradient(90deg,var(--pu),var(--cy) 50%,var(--am));-webkit-background-clip:text;background-clip:text;color:transparent;text-shadow:none;filter:drop-shadow(0 3px 10px rgba(0,0,0,.9))}
.hero h2{font-size:clamp(16px,3vw,24px);letter-spacing:.3em;font-weight:600;margin:14px 0 22px}
.hero .prop{max-width:760px;margin:0 auto;background:var(--card);border:1px solid var(--line);border-radius:14px;padding:16px 20px;text-align:left;text-shadow:none}
section{margin-top:48px}
section>h2{font-size:24px;letter-spacing:.06em;margin:0 0 18px;padding-bottom:10px;border-bottom:2px solid;border-image:linear-gradient(90deg,var(--cy),transparent) 1}
section>h2 small{color:var(--mu);font-weight:500;font-size:14px;letter-spacing:0;margin-left:8px}
.card{background:var(--card);border:1px solid var(--line);border-radius:14px;padding:20px;backdrop-filter:blur(6px);box-shadow:0 8px 30px rgba(0,0,0,.35)}
.grid{display:grid;gap:16px}.g2{grid-template-columns:repeat(auto-fit,minmax(300px,1fr))}.g3{grid-template-columns:repeat(auto-fit,minmax(260px,1fr))}
.kv p{margin:6px 0}.kv b{color:var(--cy)}
.char{border-top:4px solid var(--c)}.char h3{margin:0 0 8px;color:var(--c);letter-spacing:.05em;font-size:18px}.char p{margin:6px 0}
.ru{--c:var(--pu)}.ne{--c:var(--cy)}.ve{--c:var(--am)}
.princ{border-left:4px solid var(--cy);margin-top:16px}
h4{margin:14px 0 4px;font-size:13px;text-transform:uppercase;letter-spacing:.12em;color:var(--cy)}
.card h3 em,header h3 em{color:var(--mu);font-style:normal;font-weight:400}
ul,ol{margin:4px 0 4px;padding-left:22px}li{margin:3px 0}
.cols h3{margin:0 0 8px;font-size:16px}
.cols .conf{border-top:4px solid var(--ok)}.cols .ver{border-top:4px solid var(--wa)}.cols .pla{border-top:4px solid var(--pl)}
.rule{margin-top:16px;padding:12px 16px;border-radius:12px;background:rgba(39,225,255,.08);border:1px solid rgba(39,225,255,.3)}
.badge{display:inline-flex;align-items:center;gap:7px;font-size:12px;font-weight:700;letter-spacing:.04em;padding:3px 10px;border-radius:99px;border:1px solid currentColor;white-space:nowrap}
.badge i{width:8px;height:8px;border-radius:50%;background:currentColor}
.b-k{color:#fff;background:#16a34a;border-color:#16a34a}.b-k i{background:#fff}.b-c{color:var(--ok)}.b-v{color:var(--wa)}.b-p{color:var(--pl)}.b-d{color:var(--bl)}
.legend{display:flex;flex-wrap:wrap;gap:10px;margin:0 0 14px}
table{width:100%;border-collapse:collapse;background:var(--card);border:1px solid var(--line);border-radius:14px;overflow:hidden}
th,td{padding:10px 14px;text-align:left;border-bottom:1px solid var(--line)}th{background:rgba(39,225,255,.1);font-size:13px;text-transform:uppercase;letter-spacing:.1em}
td.n{font-weight:800;font-size:18px;width:60px;color:var(--cy)}tr:last-child td{border-bottom:0}tr:hover td{background:rgba(255,255,255,.04)}
td a{color:var(--tx)}td.n a{color:var(--cy)}
.idx{display:grid;grid-template-columns:repeat(auto-fill,minmax(210px,1fr));gap:8px;margin-bottom:22px}
.idx a{background:var(--card);border:1px solid var(--line);border-radius:10px;padding:8px 12px;color:var(--tx);font-size:14px}.idx a b{color:var(--cy);margin-right:8px}
.etapa{margin-bottom:18px;border-left:4px solid var(--cy)}
.etapa header{display:flex;align-items:center;gap:12px;flex-wrap:wrap;margin-bottom:6px}
.etapa .num{font-size:30px;font-weight:800;color:var(--cy);line-height:1}
.etapa h3{margin:0;font-size:19px;letter-spacing:.04em;flex:1 1 280px}
.blk p{margin:2px 0}
.note{margin:14px 0 0;padding:10px 14px;border-radius:10px;background:rgba(255,176,46,.1);border-left:3px solid var(--am);color:#ffe3b3}
.marco h3{font-size:17px;margin:0 0 6px}.mk{display:inline-grid;place-items:center;width:30px;height:30px;border-radius:8px;background:var(--cy);color:#001018;font-weight:800;margin-right:10px}
.marco:nth-child(1) .mk{background:var(--cy)}.marco:nth-child(2) .mk{background:var(--pu)}.marco:nth-child(3) .mk{background:var(--am)}.marco:nth-child(4) .mk{background:var(--ok)}.marco:nth-child(5) .mk{background:#fff}
.next{border:2px solid var(--cy);box-shadow:0 0 30px rgba(39,225,255,.25)}
.next .big{font-size:20px;font-weight:700;color:var(--cy)}
p.prog{margin:0 0 12px;font-size:17px}p.prog b{color:var(--ok)}
footer{margin-top:50px;text-align:center;color:var(--mu);font-size:13px}
@media(max-width:640px){.badge{white-space:normal;align-items:flex-start}.badge i{margin-top:5px;flex:none}th,td{padding:8px 8px}td.n{width:44px}.hero{padding-top:40px}.card{padding:16px}}
@media print{
 body{background:#fff;color:#000;font-size:11pt}body::before,body::after,nav.top{display:none}
 .card,table,.hero .prop,.idx a{background:#fff!important;color:#000;box-shadow:none;backdrop-filter:none;border-color:#999}
 .hero h1{background:none;color:#000;filter:none;-webkit-text-fill-color:#000}.hero{text-shadow:none;padding:10px 0}
 a,td a,td.n a,.num,h4,.kv b,.char h3,.next .big{color:#000!important}
 .note{background:#f3f3f3;color:#000}.rule{background:#f3f3f3}
 .etapa,.marco,.char,tr{break-inside:avoid}section>h2{break-after:avoid}
 .badge{color:#000!important}th{background:#eee}
}
""".replace('%BG%', bg)

page = f"""<!DOCTYPE html>
<html lang="pt-BR"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Joint Journey | Plano Mestre de Desenvolvimento</title><style>{CSS}</style></head><body>
<nav class="top" aria-label="Navegação"><a href="#topo">Início</a><a href="#visao">Visão geral</a><a href="#estado">Estado atual</a><a href="#prioridades">Prioridades</a><a href="#etapas">Etapas</a><a href="#metodo">Método</a><a href="#marcos">Marcos</a><a href="#proxima">Próxima prioridade</a></nav>
<div class="wrap">
<header class="hero" id="topo"><div class="kick">PAINEL DE DESENVOLVIMENTO</div><h1>JOINT JOURNEY</h1><h2>PLANO MESTRE DE DESENVOLVIMENTO</h2>
<div class="prop"><b style="color:var(--cy)">Propósito:</b> acompanhar o desenvolvimento do jogo, organizar prioridades e minimizar retrabalho, desperdício de tokens de IA e problemas técnicos.</div></header>

<section id="visao"><h2>2. VISÃO GERAL DO PROJETO</h2>
<div class="grid g2"><div class="card kv">
<p><b>Nome:</b> Joint Journey</p><p><b>Engine:</b> Unreal Engine 5</p>
<p><b>Ferramentas:</b> Unreal Engine 5, Blender, Claude Desktop, ChatGPT, GitHub e ferramentas auxiliares de geração 3D.</p>
<p><b>Equipe:</b> dois desenvolvedores trabalhando em computadores diferentes, com auxílio de inteligência artificial.</p></div>
<div class="card kv"><p><b>Gênero:</b> aventura cooperativa online, ficção científica, mistério, exploração, puzzles, furtividade, momentos ocasionais de ação e humor discreto.</p>
<p><b>Modo principal:</b> cooperativo para dois jogadores, Ruína e Neon.</p><p><b>Modo adicional planejado:</b> missões específicas para três jogadores, incluindo Vértice.</p></div></div>
<h3 style="margin:26px 0 12px">Personagens</h3>
<div class="grid g3">
<div class="card char ru"><h3>RUÍNA | DESTRUIÇÃO</h3><p>Destrói estruturas, remove obstáculos e disponibiliza materiais para serem reutilizados.</p></div>
<div class="card char ne"><h3>NEON | CONSTRUÇÃO E RECONSTRUÇÃO</h3><p>Reconstrói estruturas, repara mecanismos e utiliza materiais provenientes da destruição para criar novas passagens ou objetos.</p></div>
<div class="card char ve"><h3>VÉRTICE | TRANSFORMAÇÃO</h3><p>Manipula propriedades da matéria. Suas habilidades específicas ainda precisam de definição e testes.</p><p>Vértice poderá participar da narrativa do modo Duo sem exigir um terceiro personagem controlável ou inteligência artificial de companheiro.</p></div></div>
<div class="card princ"><h3 style="margin:0 0 6px">Princípio de game design</h3><p>Os puzzles precisam exigir cooperação genuína.</p><p>Não basta repetir a fórmula de destruir um objeto e restaurá-lo.</p><p>Devemos desenvolver desafios envolvendo física, comunicação, escolhas, consequências, sincronização, aproveitamento de materiais e criatividade.</p><p>As fases mais avançadas deverão conter múltiplos puzzles interligados.</p></div></section>

<section id="estado"><h2>3. ESTADO ATUAL DO DESENVOLVIMENTO</h2>
<div class="grid g3 cols">
<div class="card conf"><h3>{badge('c')} Confirmado pela equipe</h3>{ul(['Três personagens possuem modelos desenvolvidos.','Multiplayer testado entre dois computadores, com funcionamento satisfatório.','Mecânicas de caixas, botões e portas funcionando, conforme os testes relatados.','Existem salas simples de teste.','Existem protótipos de poderes antigos e portais.','GitHub sendo utilizado pela equipe para controle de versão.'])}</div>
<div class="card ver"><h3>{badge('v')} Necessita confirmação técnica</h3>{ul(['Estado final dos novos poderes de destruição e reconstrução.','Funcionamento da reutilização de materiais.','Situação da criação do novo mapa de testes.','Compatibilidade das peças 3D com o mapa.','Integridade e recuperação dos backups.'])}</div>
<div class="card pla"><h3>{badge('p')} Planejado</h3>{ul(['Primeiro puzzle completo da coluna e da ponte.','Colapso controlado do laboratório.','Campanha com múltiplos mapas.','Desafios de física, gravidade, escala e furtividade.','História, diálogos e cutscenes.','Missões extras para três jogadores.'])}</div></div>
<div class="rule"><b>Regra:</b> diferenciar sempre CONFIRMADO, EM DESENVOLVIMENTO, PLANEJADO e NÃO VERIFICADO.</div></section>

<section id="prioridades"><h2>4. PRIORIDADES DE DESENVOLVIMENTO <small>15 etapas</small></h2>
<p class="prog"><b>{NC} de 15 etapas concluídas</b>{(' &middot; atualizado em ' + ATUALIZADO_EM) if ATUALIZADO_EM else ''}</p><div class="legend">{badge('k')}{badge('c')}{badge('d')}{badge('p')}{badge('v')}</div>
<table><thead><tr><th>Nº</th><th>Etapa</th><th>Situação</th></tr></thead><tbody>{rows}</tbody></table>
<p style="margin-top:14px;color:var(--mu)">A numeração representa a ordem principal de dependências. Testes, documentação, controle de versão e monitoramento de desempenho devem acompanhar todo o desenvolvimento.</p></section>

<section id="etapas"><h2>5. DESCRIÇÃO COMPLETA DAS ETAPAS</h2>
<div class="idx">{idx}</div>{etp}</section>

<section id="metodo"><h2>6. MÉTODO OFICIAL DE DESENVOLVIMENTO</h2>
<p>A equipe seguirá um fluxo incremental:</p><div class="grid g2"><div class="card"><h3 style="margin:0">Fluxo</h3>{ul(metodo,'ol')}</div><div class="card"><h3 style="margin:0">Regras</h3>{ul(regras)}</div></div></section>

<section id="marcos"><h2>7. MARCOS DE DESENVOLVIMENTO</h2><div class="grid g2">{mc}</div></section>

<section id="proxima"><h2>8. PRÓXIMA PRIORIDADE</h2>
<div class="card next"><p class="big">Prioridade recomendada: confirmar e validar os poderes definitivos de Ruína e Neon.</p>
<p>O multiplayer já foi testado pela equipe. As interações básicas também foram relatadas como funcionais.</p>
<p>Antes de avançar para o puzzle completo, precisamos confirmar o estado real dos novos poderes.</p>
<p>Se o blockout da primeira fase já estiver sendo executado pelo Claude, concluir essa tarefa, validar seu resultado e depois reorganizar a sequência conforme as dependências restantes.</p>
<p>A equipe não deve iniciar outra implementação apenas porque existe uma nova ideia.</p>
<p><b style="color:var(--cy)">Objetivo imediato:</b> construir uma base cooperativa sólida, reutilizável e testável.</p></div></section>
<footer>JOINT JOURNEY &middot; Plano Mestre de Desenvolvimento &middot; <a href="#topo">voltar ao topo</a></footer>
</div></body></html>"""
open(S + 'JointJourney_PainelMestre.html', 'w', encoding='utf-8').write(page)
print(len(page))
