# Joint Journey | Fase 2 | Tarefa 06 | Puzzle 1: Elevador de Emergência

Autor: Henrique Collinetti (com Claude). Data: 30/10/2026. Mapa: `/Game/Maps/Fase2_Funcional`.

**Status:** o P1 funciona nos testes locais com host e cliente no mesmo PC: 3 sessões completas, incluindo papéis invertidos, mais 1 sessão gravada. O teste em **dois computadores reais está PENDENTE**.

## O que foi implementado
- **TRAVA DANIFICADA** (lado norte da casa de máquinas)
  - É um bloco de travamento preso entre duas mandíbulas sobre o trilho-guia da cabine, com corrente e a placa “TRAVA DANIFICADA”.
  - Usa o sistema de destruição já existente (`ACoopDestructible`, o tiro de poder da Ruína). Ao ser destruída, gera explosão e escombros no chão (o material liberado).
  - Regra própria: `bCanRebuild = false`. Ninguém consegue reconstruí-la.
- **SUPORTE AUSENTE** (lado sul)
  - Base e mão-francesa fixas, com um vão vazio entre elas e a placa “SUPORTE AUSENTE”.
  - O suporte começa quebrado e só o Neon pode reconstruí-lo.
  - Regras próprias: `Partner = Trava` (só reconstrói com a trava destruída) e `bCanBreak = false` (depois de pronto, não pode ser destruído).
- **Montagem:** reutiliza a animação da Fase 1 (`bAssemblyFx`, vertical). Os escombros da trava saem do chão, viram anéis verdes e se empilham até formar o suporte. Leva cerca de 1,44 s.
- **Liberação do elevador:** o GameMode, no servidor, só destrava (`ServerSetLocked(false)`) quando o suporte foi reconstruído **e** a montagem terminou. Destruir a trava não libera nada.
- **Indicadores**
  - A tela e a luz de cada console mudam de cor: vermelho = travado, âmbar = em movimento, verde = disponível.
  - Avisos para os dois jogadores: “Trava destruída: material liberado - o elevador ainda precisa do suporte” e “MECANISMO CONSERTADO: elevador LIBERADO”.
- **Dica contextual “E”:** texto discreto no centro inferior da tela, que só aparece com o jogador ao alcance do console. Usa o mesmo raio que o servidor valida.
  - Travado: “[E] Elevador TRAVADO - o mecanismo precisa de conserto”, em vermelho.
  - Disponível: “[E] Subir”, “[E] Chamar o elevador” ou “[E] Elevador aqui - entre na cabine”, em verde.
  - Em movimento: “Elevador em movimento...”.
- **Depuração:** F6–F10 agora só funcionam com o parâmetro `-F2Debug`. No jogo normal não há atalho. Nenhum teste do P1 usou depuração.

## Transferência de material (lógica real, não só efeito)
- **Regra da reconstrução:** é validada no servidor. O suporte só pode ser reconstruído se o seu `Partner` (a trava) estiver destruído. Antes disso, o servidor recusa com “Sem material”.
- **Material consumido:** a trava não pode ser reconstruída, então não há como “devolver” o material.
- **Sem segunda vez:** o suporte pronto não pode ser destruído, então não há reconstrução repetida nem consumo duplicado.
- **Efeito visual:** acompanha a lógica. Os escombros da trava somem do chão e voam para o suporte durante a montagem.

## Sincronização multiplayer
- **Servidor como autoridade:** a trava e o suporte nascem no servidor a partir de marcadores do mapa (`F2P1_Trava`, `F2P1_Suporte`) e são replicados.
- **Estados replicados:** destruído/intacto, regras e parceiro (`Partner`). Cada máquina monta a mesma animação ao receber o estado.
- **Trava do elevador:** `bLockedP1` é replicado. Consoles, dica e luzes leem esse valor localmente.
- **Liberação:** é decidida só pelo servidor, com base na hora do servidor.

## Testes (multiplayer local, host e cliente no mesmo PC)
| | Teste | Sessão 1 | Sessão 2 (reinício) | Papéis invertidos |
|---|---|---|---|---|
| A | Elevador começa travado | OK | OK | OK |
| B | Console antes do puzzle → não parte | OK | OK | OK |
| C | Neon atira na trava → intacta | OK | OK | OK |
| D | Neon reconstruir sem material → bloqueado | OK | OK | OK |
| — | Ruína atira no suporte → nada (não reconstrói) | OK | OK | OK |
| E | Ruína destrói a trava | OK | OK | OK |
| F | Elevador continua travado; console recusa | OK | OK | OK |
| G/H | Neon reconstrói; durante a montagem (+0,7 s) o elevador continua travado | OK | OK | OK |
| I | Liberado só após a montagem | 1,45 s | 1,46 s | 1,45 s |
| L | Sem duplicidade (Ruína no suporte pronto, Neon na trava) | OK | OK | OK |
| J | Embarque e subida juntos (E, servidor valida) | OK | OK | OK |
| K | Portas de B e barreiras (própria passa, a do outro bloqueia) | OK | OK | OK |
| M | Repetição em nova sessão | — | OK | OK |
| N | Fase 1 oficial (puzzle completo) e `Fase2_ElevTeste` (4 ciclos) | SUCESSO / OK | | |

- **Host e cliente concordam:** nos dois logs de cada sessão os estados batem (trava, suporte, trava do elevador).
- **Mapas preservados (SHA-1 sem alteração):** `Fase2_Blockout` `c4b76531`, `Fase2_ElevTeste` `96e8bb08` e Fase 1 oficial `b1b23af5`.
- **Dois PCs reais:** PENDENTE.

## Vídeo
**`video_P1_host_cliente.mp4`** (45 s, gravado na Unreal, host e cliente lado a lado, cerca de 5 quadros por segundo em velocidade real):
- **Host:** câmera de inspeção fixa em A, só para a gravação. Mostra o elevador travado, a trava sendo destruída, o suporte sendo montado, o console ficando verde e a subida.
- **Cliente:** câmera do jogador. Mostra embarque, subida e as barreiras de B.

`sequencia_trava_suporte_liberacao.jpg` resume em quadros a destruição, a montagem e a liberação.

## Problemas e pendências
- **Visual:** a trava e o suporte ainda são blocos (sem modelo final). A trava não “parece danificada” além da cor e da placa.
- **Mira:** os personagens precisam de linha de visão. De dentro do corredor do poço, a trava e o suporte só são alcançáveis pelos corredores laterais (esperado). Mirar de frente, através do portão, bate na cabine.
- **Escombros:** o suporte começa “quebrado” e mostra os próprios escombros no chão. Isso pode ser confundido com o material da trava. Avaliar trocar por uma marcação de “vaga vazia”.
- **Dica “Elevador aqui”:** aparece em verde no console de chamada mesmo quando a cabine já está no andar. Apertar não faz nada (o servidor responde “já está neste andar”).
- **Câmera dentro da cabine e guarda-corpo pulável:** sem mudanças nesta tarefa, conforme pedido.

## Arquivos
**Código alterado**
- `CoopDestructible.*`: regras `bCanBreak`/`bCanRebuild` e momento do fim da montagem. O padrão continua igual ao da Fase 1.
- `CoopFase2GameMode.*`: cria e controla o P1, faz a liberação e coloca a depuração atrás de `-F2Debug`.
- `CoopConsole.*`: indicador de cor e texto da dica.
- `CoopHUD.cpp`: dica “E”.
- `CoopF2Test.*`: teste automático `-F2P1` / `-F2Swap` / `-InspP1`.
- `CoopTestSubsystem.cpp`.

**Mapa e scripts**
- `MapaFase2/CriarFase2Funcional.py`: marcadores do P1, mecanismo fixo e placas.
- `Content/Maps/Fase2_Funcional.umap` (recriado).
- `F2P1Testes.bat`, `F2P1Video.bat`.

## Git e reversão
Commit “Fase 2 T06 (P1)”. Para reverter: `git revert <commit>`, que volta ao estado da Tarefa 05. Nenhum outro mapa foi alterado.
