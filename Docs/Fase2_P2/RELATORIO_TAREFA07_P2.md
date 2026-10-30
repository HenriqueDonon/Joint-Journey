# Joint Journey | Fase 2 | Tarefa 07 | Puzzle 2: A Câmara das Duas Massas

Autor: Henrique Collinetti (com Claude) · Mapa: `/Game/Maps/Fase2_Funcional` · UE 5.5 C++

Status: **IMPLEMENTADO E CONFIRMADO EM TESTE LOCAL (host + cliente no mesmo PC)**. Teste em dois PCs reais: **PENDENTE**.

## Inspeção inicial
- Git limpo a partir do commit `c71f3d8` (T06). P1 conferido em teste antes de mexer.
- Elevador (`ACoopElevator`), consoles (`ACoopConsole`), barreiras (`ACoopRoleBarrier`), portões (`ACoopGate` P2_Norte / P2_Sul / P3_Saida) e destrutíveis (`ACoopDestructible`, ServerBreak/ServerRebuild com validação de papel, alcance e linha de visão no servidor) já existiam e foram reaproveitados.
- Os antigos marcadores provisórios do P2 (placeholders, contrapeso central, rota de reencontro, rótulos) foram removidos do script do mapa; geometria das salas norte/sul do Setor B mantida.

## O mecanismo
No centro do Setor B (hub), entre as duas salas, há uma máquina de contrapesos:
- **Duas plataformas de carga** (400×400) em guias verticais, uma na sala norte (lado de Ruína) e outra na sala sul (lado de Neon), presas por **cabos e polias**: quando uma desce, a outra sobe.
- **Lingotes de massa** (blocos de 1 unidade): 3 soltos na plataforma norte, presos por **3 braçadeiras**; na plataforma sul há 1 lingote **soldado** (fixo, com faixa de solda visível).
- **Freio emperrado** (com faíscas) na sala norte: enquanto existe, a máquina não se move (fica parada fora de nível, P = −100).
- **Berço** no hub: recebe o lingote liberado da plataforma norte por uma calha fixa.
- **Calha sul** quebrada (ausente): liga o berço à plataforma sul. Só existe se Neon reconstruir.
- **Pino de travamento** ausente na sala sul: trava a máquina na posição atual.
- **Indicadores físicos**: faixas de nível (listras de alerta) nas colunas-guia na altura correta, ponteiros vermelhos nas plataformas, dois mostradores com agulha no hub visíveis pelas janelas das duas salas, e luzes de status (vermelho travado / âmbar livre / verde concluído). Nenhum texto explica o objetivo.
- **Console de rearme** em cada sala (E: "Rearmar mecanismo").

### Papel de cada personagem
- **Ruína (sala norte)**: destrói o freio (libera a máquina) e as braçadeiras (libera um lingote de cada vez). Não reconstrói nada.
- **Neon (sala sul)**: reconstrói a calha (deixa a massa passar ao lado sul) e o pino (trava). Não destrói nada.
- O material vem da destruição real: os destroços do freio surgem no freio e servem para a calha; os de uma braçadeira surgem na braçadeira e servem para o pino. A animação de montagem mostra os pedaços saindo da pilha de origem até a peça.

### Como o equilíbrio é calculado (servidor)
- MassaNorte = lingotes ainda na plataforma norte. MassaSul = lingotes na plataforma sul + 1 (soldado).
- Alvo P = clamp(100 × (MassaSul − MassaNorte), −200, +200) cm. P é a altura da plataforma sul em relação ao nível (a norte fica em −P).
- A posição se move em direção ao alvo a 60 cm/s, apenas com o freio livre e sem pino. Tudo calculado no servidor a partir do tempo do servidor; nada de física livre.
- Início: 3 × 1, freio emperrado, P = −100 (parado).

## Solução (somente no relatório)
1. Ruína destrói o freio → máquina livre; norte desce até −200 (3 × 1). Destroços no freio.
2. Neon reconstrói a calha com os destroços do freio (precisa do material de Ruína).
3. Ruína destrói **exatamente uma** braçadeira → o lingote rola ao berço e, com a calha pronta, passa ao lado sul → 2 × 2 → as plataformas sobem/descem até o **nível**.
4. Neon **espera a máquina parar no nível** (mostradores no centro, ponteiros nas faixas) e encaixa o pino com os destroços da braçadeira → trava, luz verde, portões P2_Norte e P2_Sul abrem.
- Ordem alternativa válida: braçadeira antes da calha — o lingote fica esperando no berço até a calha existir.
- **Coordenação/espera**: Neon não consegue fazer nada útil antes de Ruína soltar o freio (sem material e máquina parada); Ruína precisa saber quando parar de soltar lingotes (vê o efeito do lado de Neon pelos mostradores); o pino só encaixa com a máquina parada no nível (tolerância 6 cm) e alvo = 0.

### Erros e recuperação
- Soltar duas braçadeiras com a calha pronta → 1 × 3 → sul desce até +200; o pino é recusado. A máquina não pode ser concluída assim.
- Qualquer um aciona **Rearmar mecanismo** no console da sua sala: freio e braçadeiras voltam, calha e pino somem, pilhas de destroços são limpas, lingotes voltam ao norte e as plataformas **voltam animadas** à posição inicial. Sem reiniciar a fase. Após a conclusão o rearme fica desabilitado (conclusão permanente na tentativa).

## Sincronização
- Estado único replicado (`FBalState`: local de cada lingote, posição/tempo de referência, alvo, freio, trava, concluído, rearmes, retorno). Clientes calculam a posição visual pela mesma fórmula com o tempo do servidor, então host e cliente veem o mesmo movimento.
- Interações passam pelos RPCs já existentes (`ServerPowerShot`, `ServerInteract`) e são validadas no servidor (papel, alcance, linha de visão, `bCanBreak/bCanRebuild`, `RebuildCheck` da máquina). Peças concluídas ficam indestrutíveis/irreconstruíveis → sem ativação duplicada nem consumo duplo de material.

## Ajustes do P1
- **A**: o suporte reconstruível não mostra mais destroços próprios (`bShowOwnDebris = false`); os destroços aparecem só na trava destruída e a montagem os leva até o suporte.
- **B**: consoles de chamada mostram o estado real: verde só quando a ação tem efeito ("[E] Chamar o elevador"); cinza quando o elevador já está no andar ou em movimento; vermelho quando travado.
- Mecanismos do P1 continuam nos corredores laterais; nenhuma pista nova.

## Testes (multiplayer local, host + cliente no mesmo PC)
Fluxo automatizado `-F2P2` com os dois jogadores agindo por RPC real (logs em `logs/`).

| Item | n1 | swap + 100 ms | n2 | n3 |
|---|---|---|---|---|
| A. P1 continua funcionando | OK | OK | OK | OK |
| B. Elevador leva os dois a B | OK | OK | OK | OK |
| C. Barreiras separam | OK | OK | OK | OK |
| D. Só mecanismos autorizados (Neon sem material, pino desalinhado, Ruína atirando na calha pela parede) | OK | OK | OK | OK |
| E. Plataformas respondem à carga | OK | OK | OK | OK |
| F. Ação de um visto pelo outro | OK | OK | OK | OK |
| G. Não conclui com um só | OK | OK | OK | OK |
| H. Equilíbrio (pino só no nível) | OK | OK | OK | OK |
| I. Portões só após conclusão | OK | OK | OK | OK |
| J. Reencontro | OK | OK | OK | OK |
| K. Erro + rearme sem soft lock | OK | OK | OK | OK |
| L. Sincronização host/cliente | OK | OK | OK | OK |
| M. Nova tentativa restaura o estado | OK (rearme restaura tudo; nova sessão inicia limpa) |
| N. Fase 1 e sistemas anteriores | ver seção Regressões |

- "swap": host = Ruína, cliente = Neon, com `-F2Lag=100` (latência simulada).
- n3 e a sessão de vídeo confirmaram o retorno animado das plataformas no rearme (P 198 → −100 em ~5 s).

### Regressões (lote 2, código final)
- P1 isolado (`-F2P1`, host + cliente): trava/suporte/montagem/liberação OK; console mostra "Elevador neste andar" em cinza quando não há efeito (ajuste B confirmado); barreiras OK.
- `Fase2_ElevTeste` (elevador + barreiras, 4 ciclos por papel): todos OK, 0 falhas.
- Fase 1 oficial (fluxo coluna/ponte/túnel): SUCESSO, 2 de 2 jogadores no fim.
- `Fase2_Blockout`, `Fase2_ElevTeste` e Fase 1: sem alterações no Git.

## Vídeo e capturas
- `video_P2_host_cliente.mp4`: host (esquerda) e cliente (direita) lado a lado, quadros reais da Unreal: chegada a B, separação, Ruína no norte, Neon no sul, movimento das plataformas, erro (2 lingotes), rearme, solução correta, travamento, portões e reencontro.
- `capturas/`: vistas fixas do P2 (sala norte, sala sul, hub pela janela, freio/braçadeiras, calha/pino, planta B).

## Problemas e pendências
- **Teste em dois PCs reais: pendente** (precisa da equipe).
- Modelos e materiais são provisórios (primitivas); sem som.
- O vídeo é capturado em quadros (≈5 qps), não em tempo real de jogo.
- O tempo de rearme depende da distância (até ~7 s); durante o retorno a máquina pode ser liberada de novo — comportamento consistente, mas vale avaliar em jogo.
- Legibilidade dos mostradores pela janela precisa de validação com jogadores reais (sem dicas textuais).

## Arquivos
Novos: `Source/JogoCoop/CoopBalanceMachine.h/.cpp`, `F2P2Testes.bat`, `F2P2Um.bat`, `F2P2Lote2.bat`, `F2Compilar.bat`.
Modificados: `CoopDestructible.h/.cpp` (bShowOwnDebris, RebuildCheck), `CoopConsole.h/.cpp` (MachineReset, prompt com estado), `CoopHUD.cpp` (cor do prompt), `CoopFase2GameMode.cpp` (ajuste A do P1), `CoopF2Test.h/.cpp` e `CoopTestSubsystem.cpp` (fluxo P2 e vistas), `MapaFase2/CriarFase2Funcional.py`, `Content/Maps/Fase2_Funcional.umap`.
Não modificados (SHA verificado): Fase 1, `Fase2_Blockout`, `Fase2_ElevTeste`.

## Git e reversão
GIT_AQUI
