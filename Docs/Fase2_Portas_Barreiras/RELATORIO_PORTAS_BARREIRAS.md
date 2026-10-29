# Joint Journey | Fase 2 | Tarefa 04 | Portas opostas e barreiras seletivas (protótipo)

Status: **APROVADO EM TESTE LOCAL** (host + cliente na MESMA máquina, loopback; uma rodada com 100 ms de latência artificial no cliente). **Teste em 2 computadores reais: PENDENTE.**
Mapa: `/Game/Maps/Fase2_ElevTeste` (protótipo). Fase2_Blockout e Fase 1 não foram alterados.

## Implementado
- **Portas no elevador (`ACoopElevator`)**: duas portas laterais opostas (+Y e -Y), um portão de entrada (-X) e uma parede fixa (+X). Painéis sobem 250 cm para abrir (0,35 s). Laterais abrem só com a cabine PARADA no andar de separação (topo; `bSideDoorsOpenAtTop`); portão abre só parada embaixo (`bFrontGateOpenAtBottom`). Durante o movimento tudo fica fechado.
- **Barreira seletiva reutilizável (`ACoopRoleBarrier`)**, independente do elevador: volume translúcido que bloqueia o canal Pawn; o papel autorizado (`AllowedRole`) a atravessa. Roxa = Ruína (corredor +Y), ciano = Neon (corredor -Y).
- **Espaços de teste**: dois corredores provisórios (250 x 665 cm) no andar de separação, com a barreira 80 cm após cada porta.
- **Driver de teste** (`ACoopElevTest`, só com `-ElevAuto`): roteiro automático por personagem, incluindo papéis invertidos (`-ElevSwap`, via a RPC de teste já existente `ServerSetRole`, chamada por reflexão).

## Como funciona
- **Portas:** estado alvo calculado da mesma forma em todas as máquinas a partir do comando replicado do elevador (destino + hora de partida na hora do servidor). O servidor dá a partida com 0,6 s de antecedência: as portas fecham nesse intervalo, antes de a cabine se mover. Os painéis ficam a 125 cm do centro (fora da cabine) e a partida exige todos com o centro a até 90 cm do centro da cabine; portanto um painel nunca fecha sobre um personagem.
- **Colisão seletiva:** a malha bloqueia só `Pawn` (não bloqueia mira, câmera nem fragmentos). A cada 0,2 s, em cada máquina (servidor e clientes), a barreira faz a cápsula do personagem autorizado ignorá-la (`IgnoreActorWhenMoving`), com base no papel replicado (`CoopRole`). Servidor e predição do cliente seguem a mesma regra, qualquer que seja o host.
- **Reutilização:** colocar `CoopRoleBarrier` em qualquer mapa e definir `AllowedRole`, `Size` (cm), opcionalmente `Tint`/`Opacity` (`bAutoTint` = cor padrão por papel). Material: `/Game/Fase2/Materiais/M_F2_Barreira` (parâmetros `Cor`, `Opac`).

## Conflito encontrado no acionamento automático (e correção mínima)
- **Antes:** o elevador só rearmava quando a plataforma ficava totalmente vazia. Se um personagem ficasse a bordo no andar de separação, o outro voltava e o elevador **nunca** partia (retorno bloqueado). Conclusão por leitura do código; não foi reproduzido em teste.
- **Correção (no `ACoopElevator`):** rearma quando **alguém** sai (`N < P`). Comprovado no ciclo 4: Neon fica a bordo, Ruína vai, volta, e o elevador desce.
- **Outros ajustes mínimos:** área "a bordo" 110 → 90 cm (as cápsulas não encostam nas portas) e antecedência 0,4 → 0,6 s (portas fecham antes de mover).
- **Risco remanescente (não corrigido, fora do escopo):** se os dois ficarem juntos dentro da cabine no andar de separação por 1,5 s, ela desce mesmo que não fosse a intenção. Para o jogo final é recomendável um acionamento explícito (botão/console).

## Testes (local; NÃO é teste em 2 PCs)
| Execução | Papéis | Latência | Ciclos | Resultado |
|---|---|---|---|---|
| 1 | host = Neon, cliente = Ruína | 0 | 4 | todos OK |
| 2 | host = Ruína, cliente = Neon (invertidos) | 100 ms (cliente) | 4 | todos OK |

Ciclos: 1 rota certa, 2 rota errada, 3 rota certa (repetição), 4 Neon fica a bordo.
1. Subida juntos: ambos com base no elevador, altura relativa 90,1–90,2 cm, deriva horizontal 0,0 cm. **OK**
2. Parada no topo e 3. portas abrindo: 0,6–0,7 s após parar, nas duas máquinas. **OK**
4. Ruína atravessou a roxa (|Y| até 651–664) e foi bloqueada pela ciano (parou em |Y| 181; a barreira está em 215). **OK**
5. Neon atravessou a ciano (650–663) e foi bloqueado pela roxa (181). **OK**
6. Ambos retornaram ao elevador por seus caminhos em todos os ciclos. **OK**
7. Ninguém preso nas portas; em ~1.700 amostras de movimento, 0 com porta aberta. **OK**
8. Elevador funcional em todos os ciclos. **OK**
9. Repetição: ciclos 1 e 3, mais a 2ª execução completa. **OK**

Sincronização da altura da cabine entre host e cliente: média 0,35 cm e máx. 1,2 cm em movimento; ≤0,5 cm parado.

Evidências: `video_host_Neon_cliente_Ruina_2x.mp4` (execução 1, lado a lado, acelerado 2x), `quadros_separacao.jpg`, logs `log_run1_normal_*.txt`, `log_run2_swap_lag100_*.txt`, análise `MapaFase2/analise_elev_log.py`.

## Pendências
- Teste em 2 computadores reais; perda de pacotes; jogador entrando ou saindo no meio da viagem; pulo junto às portas.
- Partida involuntária se os dois se reunirem na cabine no andar de separação (ver acima).
- Vértice é bloqueado pelas duas barreiras (por definição); não testado.

## Git / reversão
Commit desta tarefa (ver `git log`). Reverter só esta tarefa: `git revert <commit>`, que volta o elevador e o mapa à versão da Tarefa 03 (`9806769`).
