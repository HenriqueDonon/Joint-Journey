# Joint Journey | Fase 1 (Fase1_Arquitetura) | Coluna estrutural, colapso progressivo e derrota visível

Status:
- **Testes funcionais:** passaram nos testes locais (host + cliente na mesma máquina).
- **Teste em 2 computadores:** PENDENTE.
- **Visual:** melhorou de forma clara, mas há limitações declaradas abaixo.

Mapas preservados (SHA-1): `Fase1_Editavel` `f3b9c861…`, `Fase1_Experimental` `cd3b3e2f…`, `Fase2_Blockout` `c4b76531…`.

## Inspeção
- **Coluna:** `Coluna_Inteira` com 616 cm de altura e 10 fragmentos, sem alteração. É a mesma peça destrutível, replicada e com material compartilhado com a ponte.
- **Antes:** a coluna só tocava um deck pequeno (painéis do teto a 6,2 m), sem ligação com as treliças do galpão, que ficam a ~15,4–18 m.
- **Tempo real do colapso:** `CollapseSecs = 12 s` no código. Não foi alterado.
- **Fluxo anterior:**
  - contagem com rachaduras;
  - queda do teto em 0,8 s;
  - reinício 1,4 s depois, cedo demais para ver o impacto.

## O que foi modificado
- **Novo `ACoopCollapseRig`** (`Source/JogoCoop/CoopCollapseRig.h/.cpp`): ator colocado só no `Fase1_Arquitetura`.
  - Monta em BeginPlay, em cada máquina, a estrutura sustentada e todos os efeitos.
  - Não tem colisão (não prende nem empurra jogadores).
  - Replica apenas o estado: estágio, hora de início, duração e início da queda/estabilização. O visual é calculado igual em servidor e clientes a partir da hora do servidor.
- **`CoopFase1GameMode`** (só quando o mapa tem o rig, ou seja, só no layout 2):
  - inicia a instabilidade quando a coluna cai;
  - estabiliza se o Neon reconstrói;
  - inicia a queda no fim dos 12 s;
  - esmaga os jogadores **no instante do impacto visível** e reinicia **2,5 s depois do impacto** (≈4,1 s após o início da queda).
  - Sem o rig, o código antigo segue igual. O deck simples (`F1L_Teto`) só é criado quando não há rig.
- **`CoopTestSubsystem`** (só teste):
  - variante `-F1Salvar`, que reconstrói aos 8,0 s e aos 11,6 s;
  - câmera de inspeção `-InspCam` no host;
  - tour "estrutura".
- **Mapa e script:** `MapaFase1Arq/CriarFase1Arquitetura.py` agora coloca o rig. `Content/Maps/Fase1_Arquitetura.umap` foi regenerado.
- **Inalterados:** poderes, transferência de material, regras de reconstrução, condições de vitória e derrota (apenas o momento do esmagamento acompanha o impacto) e a arquitetura principal.

## Como a coluna sustenta o teto
1. **No topo da coluna:** um capitel de aço com faixa de perigo, sob a viga principal do mezanino (o deck de painéis, agora com moldura de vigas).
2. **Torre de transferência:** do capitel sobe uma torre de aço com 4 montantes e contraventamento em X nas 4 faces, mais mãos-francesas.
3. **Seção do teto:** a torre sustenta uma seção pesada do teto (laje de 15,5×12,5 m, 3 vigas principais e 5 travessas).
4. **Ligações:** a seção liga-se às treliças principais do galpão por 6 montantes, tem 3 consoles de ancoragem na parede e pendura o mezanino por 4 tirantes.
5. **Iluminação de serviço:** fria, para que a estrutura seja legível.

## Colapso progressivo (12 s reais)
- **Fase A** (logo após a destruição):
  - vibração leve de toda a estrutura sustentada;
  - o mezanino cede e inclina sem a coluna (até 40 cm e 3°);
  - as primeiras rachaduras emissivas no capitel e nas juntas, mais as rachaduras na parede (sistema existente);
  - poeira escorrendo das juntas;
  - as luzes de serviço começam a piscar.
- **Fase B** (agravamento):
  - vibração e rachaduras crescem (18 rachaduras com ordem e crescimento progressivos);
  - poeira mais intensa;
  - rajadas de faíscas nas conexões, cada vez mais frequentes;
  - 36 detritos caindo da laje, com densidade crescente;
  - 6 peças secundárias (chapas, travessas, bandejas) se soltam aos 42/55/66/76/85/93% do prazo e caem até o piso;
  - as luzes ficam âmbar-avermelhadas e piscam mais rápido.
- Tudo fica concentrado na área da estrutura sustentada.
- **Câmera:** o jogador próximo sente um tremor leve, que é local, só visual.

## Reconstrução a tempo
- O servidor muda o estágio para "estabilizando".
- Vibração, cedência e rachaduras decaem em 1,2 s e a estrutura volta à posição.
- Poeira, faíscas e queda de detritos param; detritos que ainda não tinham caído são cancelados.
- O que já caiu fica no chão como entulho até o reinício.
- A queda final só acontece por ordem explícita do servidor, portanto uma reconstrução perto do prazo não causa queda atrasada nos clientes.

## Desabamento final e derrota
- No fim dos 12 s, o mezanino, a torre, a seção do teto e as vigas caem em queda livre (1,6 s) com inclinação; a torre se esmaga entre o deck e a laje.
- **No impacto:** nuvem de poeira no chão, faíscas, clarão e tremor forte.
- O esmagamento ocorre no impacto, e o reinício acontece 2,5 s depois, com a coluna inteira, a ponte destruída e todos no início.

## Testes (locais: host = Neon, cliente = Ruína)
| Teste | Resultado |
|---|---|
| A. Ruína destrói a coluna | OK |
| B/C. Instabilidade e progressão durante o prazo | OK (log + vídeo V2/V3) |
| D/E. Neon reconstrói aos 8,0 s → efeitos param | OK |
| F. Reconstrução aos 11,6–11,7 s (limite 12) → sem queda/esmagamento atrasado | OK (2 execuções) |
| G. Sem reconstrução → seção desaba e derrota | OK |
| H. Reinício restaura (coluna inteira, ponte destruída, todos no início) | OK |
| I. Puzzle cooperativo completo no novo mapa | SUCESSO |
| J. Mapa oficial `Fase1_Editavel` com o mesmo código | SUCESSO (comportamento original) |
| K. Host x cliente | estados iguais nos dois logs; visual calculado igual pelos dois |

Pendente: teste em 2 PCs reais; medição de desempenho (FPS).

## Vídeos (gravados na Unreal)
- `V1_coluna_estrutura.mp4`: passeio de câmera, 30 fps, da coluna até as treliças.
- `V2_destruicao_e_salvamento.mp4`: câmera de inspeção, velocidade normal. Mostra destruição, instabilidade crescente, salvamento aos 8 s e um segundo ciclo salvo aos 11,6 s.
- `V3_derrota_desabamento.mp4`: câmera de inspeção, velocidade normal, sem cortes. A sequência completa até o reinício.
- `V3b_derrota_camera_personagem.mp4`: a mesma derrota pela câmera do personagem (cliente).
- `V4_puzzle_cooperativo_host_cliente.mp4`: host e cliente lado a lado, velocidade normal, a ~4 quadros por segundo.

Os vídeos das câmeras de jogo têm ~8 e ~4 quadros por segundo, porque a captura de tela por quadro com duas instâncias abertas é limitada. O movimento fica "travado", mas a velocidade é real.

## Avaliação crítica (critérios de aprovação visual)
- **A coluna parece sustentar o teto?** Sim. A torre e as ligações até as treliças estão legíveis no V1.
- **Conexões naturais?** Razoáveis: são blocos de aço provisórios, sem modelagem detalhada.
- **O deck deixou de ser peça isolada?** Sim, agora pendurado e apoiado na torre.
- **Risco imediato e progressão perceptíveis?** Peças e detritos caindo, faíscas, rachaduras e luzes piscando são perceptíveis. A **vibração e a cedência são sutis** à distância da câmera de inspeção.
- **Desabamento grande e visível antes do reinício?** Sim: a laje inteira cai sobre a área dos jogadores e fica 2,5 s visível.
- **Limitações visuais:**
  - a poeira usa esferas translúcidas simples (sem Niagara), e partículas de verdade melhorariam muito;
  - não há som (o projeto não tem efeitos sonoros adequados);
  - a laje e as vigas usam cores lisas, e a face superior da laje fica muito clara sob os refletores;
  - as rachaduras são faixas emissivas simples.

## Git / reversão
Ver o commit desta tarefa. Reverter: `git revert <commit>`.
- O rig existe só no `Fase1_Arquitetura`.
- Sem rig, o GameMode usa o comportamento anterior.
