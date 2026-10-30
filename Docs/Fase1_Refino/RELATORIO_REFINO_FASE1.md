# Joint Journey | Fase 1 (Fase1_Arquitetura) | Refino: colapso, derrota, construção da ponte e túnel

Autor: Henrique Collinetti, com Claude. Mapa trabalhado: `/Game/Maps/Fase1_Arquitetura`.

Mapas preservados (SHA-1 sem alteração):
- `Fase1_Editavel` `f3b9c861…`
- `Fase1_Experimental` `cd3b3e2f…`
- `Fase2_Blockout` `c4b76531…`
- `Fase2_ElevTeste` `96e8bb08…`

## Status
- Funcional: passou nos testes multiplayer locais (host e cliente no mesmo PC).
- Teste em dois computadores reais: **PENDENTE**.
- Visual: melhora clara, com limitações listadas abaixo.

## O que foi alterado
**`ACoopCollapseRig`** (`CoopCollapseRig.*`)
- **Estrutura maior:** a seção sustentada agora cobre toda a área inicial, de parede a parede (14,9 × 28 m), logo abaixo das treliças (13,8 m).
- **Viga de transferência:** nova viga com faixa de perigo, que atravessa o galpão e se apoia na torre sobre a coluna.
- **Ligações:** mais travessas, 8 montantes até as treliças e ancoragens nas duas paredes.
- **Luminárias:** 6 luminárias penduradas iluminam a área inicial; piscam, avermelham e caem junto com a seção.
- **Segunda onda de queda:** 7 vigas do vão central despencam logo depois, espalhando o colapso além da seção.
- **Câmera de derrota:** câmera fixa no próprio ator (replicado), com tremor na queda e no impacto.
- **Poeira de impacto:** nuvem maior, cobrindo toda a área soterrada; a seção agora fica a 1,4 m do piso.

**`ACoopCharacter`**
- Novo estado replicado **`bBrace`**: o personagem para, fica sem poderes e se encolhe para se proteger.
- **`SetCrushed(b, bBury)`** com soterramento: o personagem fica oculto sob os destroços (`bHidden` replicado).

**`ACoopFase1GameMode`** (só quando o mapa tem o rig)
- **No início da queda:** todos os personagens se encolhem, o movimento é bloqueado (`ClientIgnoreMoveInput`) e a câmera passa para a câmera de derrota (`SetViewTargetWithBlend`, 0,45 s).
- **No impacto:** os personagens são soterrados (ocultos) e aparece a mensagem "FALHA: a estrutura desabou sobre voces".
- **2,8 s depois do impacto:** reset sem animação. Coluna inteira, ponte destruída, todos no início, câmera e controle devolvidos.
- **Túnel:** no layout 2 da arquitetura, deixa de criar os adereços antigos e as luzes antigas.

**`ACoopDestructible`**
- **Animação de montagem na reconstrução** (`bAssemblyFx`), ativada só no `Fase1_Arquitetura`. O mapa oficial segue igual.
- **Destroços consumidos:** os destroços do parceiro (fragmentos da coluna ou pedaços da ponte no poço) são recolhidos e voam até a peça reconstruída. Isso cumpre o pré-requisito "destroços → outro objeto".
- **Rede:** `Partner` agora é replicado. Novo `bQuietChange` faz o reset sem efeitos.

**Novo `ACoopTunnelDressing`** (revestimento do túnel)
- Chapas de aço escuro com nervuras, rodapé pintado e faixa ciano na mesma altura do galpão.
- Piso de concreto escuro com faixas de perigo e guias ciano.
- Teto com vigas, canos e bandeja de cabos.
- Moldura de perigo no portal e na porta, consoles com tela ciano.
- Luz fria da mesma família do galpão, com acentos âmbar no portal.

**Mapa e testes**
- Novo `MapaFase1Arq/Refino.py`: ajusta o rig e coloca o revestimento sem recriar o mapa. `CriarFase1Arquitetura.py` foi atualizado com os mesmos valores.
- `CoopTestSubsystem`: novo tour "tunel", câmera de inspeção da ponte (`-InspPonte`), câmera de inspeção que cede a vista para a câmera de derrota, e o roteiro espera 1,6 s antes de atravessar a ponte.

## Como fica o jogo
- **Pilar:** a coluna sustenta a torre e a viga de transferência, que segura toda a cobertura da área inicial. Se ela cai, cai o "teto da sala inteira", e não mais um "tetinho".
- **Colapso progressivo (12 s, sem mudança no tempo):**
  - aumentam ao longo dos 12 s: vibração, cedência, rachaduras, poeira das juntas, faíscas, detritos e 6 peças que se soltam;
  - as luzes piscam e ficam âmbar;
  - nada disso tem colisão, então não mata, não bloqueia e não interfere no puzzle.
- **Derrota (lose scene, ~4,4 s):** câmera ampla mostra a seção inteira caindo, com a segunda onda no vão. No impacto vêm tremor, clarão e poeira, os personagens somem soterrados e aparece o aviso; depois, reset. Não há mais personagens em pé durante o desmoronamento.
- **Ponte (horizontal, ciano, ~1,3 s):**
  - um feixe-guia atravessa o vão;
  - 11 segmentos saem dos destroços da coluna, fazem um arco e encaixam em sequência, da margem inicial até a final;
  - depois aparece o modelo real da ponte.
- **Coluna (vertical, verde, ~1,4 s):** um pilar de energia sobe da base e 7 anéis vêm dos destroços da ponte no poço. Eles sobem girando e se empilham de baixo para cima, com um pulso horizontal a cada encaixe.
- **Colisão:** vale imediatamente em ambas as estruturas, para não mudar regras nem tempos do puzzle.

## Testes (A–M)
| Item | Resultado | Tipo |
|---|---|---|
| A. Destruição do pilar | OK | multiplayer local |
| B. Instabilidade começa após a destruição | OK (vídeos V1/V2) | multiplayer local |
| C. Elementos menores não bloqueiam nem matam | OK, sem colisão; nenhum esmagamento nos ciclos salvos | multiplayer local |
| D. Neon constrói a ponte com a nova animação | OK (V3) | multiplayer local |
| E. Ponte funcional após a animação | OK, travessia dos dois | multiplayer local |
| F. Neon reconstrói a coluna | OK | multiplayer local |
| G. Reconstrução interrompe o colapso | OK aos 8,10 s e aos 11,76 s (limite de 12 s), sem queda depois | multiplayer local |
| H. Derrota em estado controlado | OK: câmera de derrota, controle bloqueado, aviso | multiplayer local (host e cliente) |
| I. Personagens não ficam em pé durante o desmoronamento | OK: encolhidos e depois ocultos (V1b) | multiplayer local |
| J. Reset | SUCESSO (coluna inteira, ponte destruída, todos no início) | host e cliente |
| K. Túnel harmonizado | OK (V4) | local |
| L. Puzzle cooperativo completo | SUCESSO (2 de 2 no fim do túnel), em 2 execuções | multiplayer local |
| M. Mapa oficial `Fase1_Editavel` | SUCESSO (2 de 2) | multiplayer local |

Testes em dois computadores reais: PENDENTES.

## Vídeos
| Vídeo | Conteúdo | Câmera e cadência |
|---|---|---|
| V1 | Passeio da estrutura, depois destruição, instabilidade, colapso, cena de derrota e reset | Passeio a 30 fps; jogo pela câmera de inspeção, que entrega a vista para a câmera de derrota do jogo, ~8 fps em velocidade real |
| V1b | A mesma derrota pela câmera do jogador cliente | ~4 fps em velocidade real |
| V2 | Salvamento em 2 ciclos (8,1 s e 11,8 s) e montagem vertical da coluna | ~8 fps |
| V3 | Construção da ponte, travessia, destruição da ponte e reconstrução da coluna a partir dos destroços do poço | ~8 fps |
| V4 | Túnel pós-fase | 30 fps |
| V5 | Host e cliente lado a lado completando o puzzle | ~4 fps |

Os vídeos das câmeras de jogo saem em ~4–8 fps por causa da captura com duas instâncias no mesmo PC: o movimento fica travado, mas a velocidade é real.

## Avaliação crítica e limitações
- **Pilar e escala:** agora a leitura é de que ele sustenta a área inteira. O mezanino sobre a coluna continua baixo e pequeno (6,2 m); o que ficou grande foi a cobertura acima dele.
- **Poeira:** a poeira do impacto usa esferas translúcidas simples; de perto aparecem como "discos". Sistema de partículas (Niagara) seria a melhora natural.
- **Seção caída:** vista de cima, a face superior da laje parece um piso cinza-claro liso. Comunica o "achatamento", mas falta entulho irregular.
- **Personagens soterrados:** ficam ocultos, sem animação de queda (o projeto não tem animações de derrota).
- **Peças de montagem:** são blocos emissivos e saturam em branco/verde nos vídeos; a intensidade pode ser ajustada após avaliação.
- **Colisão imediata:** por ~1 s pode-se pisar na ponte antes de ela estar visualmente completa (decisão para não alterar o ritmo).
- **Túnel:** coerente com o galpão, mas ainda simples (blocos e materiais do galpão, sem modelos novos). A porta fecha antes da entrada no túnel, então a continuidade aparece em V5.
- **Som:** sem som.
- **Detritos do colapso:** atravessáveis (sem colisão, por segurança do puzzle).

## Pendências
- Teste em 2 PCs.
- Medição de FPS.
- Ajuste fino de intensidade das animações e da poeira.
- Sons.

## Git e reversão
Ver o commit "Fase1_Arquitetura refino". Reverter com `git revert <commit>`.
- O mapa oficial e os outros não usam o rig nem `bAssemblyFx`, então não são afetados.
- Assets novos: nenhum. Só código, o mapa `Fase1_Arquitetura` e scripts.
