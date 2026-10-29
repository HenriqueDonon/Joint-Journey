# Joint Journey | Fase 2 | Tarefa 03 | Elevador multiplayer (protótipo isolado)

Status: **APROVADO EM TESTE LOCAL (2 processos na mesma máquina, loopback)**. **Teste real em 2 computadores: PENDENTE DE VALIDAÇÃO.**

## O que foi implementado
- `ACoopElevator`: plataforma 250x250x40 cm que sobe/desce 600 cm (5 s, movimento suavizado), transportando os jogadores.
- Mapa isolado `/Game/Maps/Fase2_ElevTeste` (Fase2_Blockout e Fase 1 não foram tocados).
- Driver de teste `ACoopElevTest` (só ativa com `-ElevAuto`): cada instância faz o personagem local caminhar, embarcar, subir/descer 3 ciclos e gravar log + capturas.

## Como funciona
- **Acionamento:** quando TODOS os jogadores da sessão estão sobre a plataforma por 1,5 s, o servidor inverte o destino (base<->topo). Só parte de novo depois que a plataforma ficar vazia (permite entrar/sair com o elevador parado).
- **Movimento:** kinemático (componente Movable); os personagens o usam como base de movimento (CharacterMovement), sem escorregar.
- **Replicação:** só o comando é replicado (`FElevCmd {bUp, T0}`, T0 = hora do servidor, com 0,4 s de antecedência). A altura é função determinística da hora do servidor (`GameState->GetServerWorldTimeSeconds`), calculada igualmente em servidor e clientes; sem replicar posição por frame. Autoridade: apenas o servidor decide e altera o comando. Cliente que entra depois calcula a posição certa a partir do comando.
- Fora do escopo (não feito): paredes coloridas, portas, puzzles.

## Arquivos
Novos: `Source/JogoCoop/CoopElevator.h/.cpp`, `CoopElevTest.h/.cpp`; `MapaFase2/CriarFase2ElevTeste.py`; `CriarFase2ElevTeste.bat`; `Fase2ElevTeste.bat`; `Content/Maps/Fase2_ElevTeste.umap`; `Docs/Fase2_Elevador/*`.
Alterados: nenhum arquivo existente.

## Testes (host = Neon, cliente = Ruína; mesma máquina, loopback; NÃO é teste em 2 PCs)
| Execução | Condição | Ciclos | Resultado |
|---|---|---|---|
| 1 | sem latência | 3 (base->topo->base) | OK |
| 2 | cliente com `Net PktLag=100` (100 ms) | 3 | OK |

Verificações (logs 4x/s + vídeo):
1. Ruína e Neon no mesmo elevador: ambos embarcaram (base de movimento = `CoopElevator_0` nos dois, em ambas as máquinas). **OK**
2. Subida juntos: altura do personagem em relação à plataforma constante, 90,0–90,2 cm; deriva horizontal 0,0 cm. **OK**
3. Descida juntos: idem. **OK**
4. Entrada/saída com elevador parado (embarque, saída 420 cm para +X no topo e -X embaixo): **OK**
5. Sincronização host x cliente (altura da plataforma na mesma hora do servidor): movimento: média 1,8–2,0 cm, máx. 5,5 cm (≈0,9 % do curso; inclui erro da amostragem a 4 Hz); parado: 0,0–0,3 cm. Personagem remoto visto no cliente também com rel.Z = 90. **OK**
6. 3 ciclos completos em cada execução. **OK**

Evidências: `video_teste_host_cliente_sem_lag.mp4` (host e cliente lado a lado, ~4 fps), `quadros_host_cliente.jpg`, `log_run2_lag100_*.txt`. O log bruto da execução 1 foi sobrescrito por engano ao copiar; seus números acima vêm da análise feita antes disso, o vídeo permanece.

## Não testado / pendências
- **2 computadores reais** (latência/perda reais) — PENDENTE.
- Entrada de jogador durante o movimento, saída de jogador durante a viagem, 3 jogadores, pulo dentro da cabine, borda da plataforma, perda de pacotes.
- Integração no `Fase2_Blockout` (não feita, por escopo).
- `PktLag` afeta pacotes enviados pelo cliente; não substitui rede real.
- Ao deixar o elevador sem ninguém a bordo ele permanece no último destino (sem retorno automático).

## Git / reversão
Ver commit desta tarefa no log. Reverter: `git revert <commit>` ou apagar os arquivos listados (nenhum arquivo existente foi alterado).
