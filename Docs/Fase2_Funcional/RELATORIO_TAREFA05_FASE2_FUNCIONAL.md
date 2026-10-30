# Joint Journey | Fase 2 | Tarefa 05 | Redesenho funcional do Poço dos Contrapesos

Autor: Henrique Collinetti (com Claude). Data: 30/10/2026.

**Status**
- **Base espacial:** a base nova está montada no mapa experimental.
- **Integração:** elevador, portas e barreiras estão integrados.
- **Testes:** o percurso completo A → B → reencontro → C → D passou nos testes locais de multiplayer (host e cliente no mesmo PC).
- **Dois computadores reais:** teste **PENDENTE**.
- **Puzzles:** **P1, P2 e P3 NÃO estão implementados**. O mapa só tem as áreas reservadas e os placeholders.

## 1. Mapa experimental
- **Mapa:** `/Game/Maps/Fase2_Funcional` (`Content/Maps/Fase2_Funcional.umap`).
- **Script gerador:** `MapaFase2/CriarFase2Funcional.py`. Para recriar o mapa, crie o arquivo `MapaFase2/RECRIAR_FUNC.txt` e rode `F2FuncCriar.bat`.
- **Mapas preservados (SHA-1 sem alteração):**
  - `Fase2_Blockout`: `c4b76531`
  - `Fase2_ElevTeste`: `96e8bb08`
  - Fase 1 oficial: `b1b23af5`
  - Backup da Fase 1: `0440ea46`

## 2. Dimensões reais (cm)
**Base de referência**
- **Alturas:** foram mantidas — A = 0, B = 600, C = 1200, D = 1800.
- **Poço central:** foi mantido (500 × 500, X 950–1450, Y −250..250).
- **Envoltório:** X −600..2800 e Y −1250..1250 (34 × 25 m), paredes até 2600. O blockout original tinha 24 × 18 m.
- **Por que ampliar:** as salas separadas de B, o reencontro e as escadas não cabiam.
- **Personagem:** cápsula de raio 34 e meia-altura 88. Pulo de 650 cm/s, o que dá cerca de 215 cm de altura e cerca de 8 m de alcance horizontal no mesmo nível.
- **Consequência do pulo:** os desníveis de 600 cm não podem ser pulados para cima. A separação dentro de um mesmo nível é feita com paredes, não com vãos.

| Setor | Área útil | Observação |
|---|---|---|
| A — saguão de início | 10,5 × 25 m (X −600..450) | open até o teto (vê a altura do poço) |
| A — corredor do poço / P1 | 5,9 × 6 m (X 450..1040, Y ±300) | leva ao portão da cabine |
| Cabine A↔B | 2,5 × 2,5 m, curso 600 cm em 5 s | casa de máquinas 3,4 × 3,4 m + contrapeso a leste |
| B — sala norte (Ruína) | 15 × 9,3 m, pé-direito 6 m | acesso pela porta +Y da cabine, barreira roxa |
| B — sala sul (Neon) | 15 × 9,3 m, pé-direito 6 m | acesso pela porta −Y da cabine, barreira ciano |
| B — hub de contrapeso | 5 × 6 m (fechado) | visto pelas janelas das duas salas |
| B — reencontro | 8,3 × 25 m | portões P2_Norte/P2_Sul (fechados até o P2) |
| Escada B→C | 2,5 m de largura, 20 degraus de 30 × 50 (31°) | dentro do reencontro |
| C — faixa norte | 23,5 × 10 m | circulação + portão P3 |
| C — cobertura sul (P3) | 15 × 9,5 m | trajetória da esfera |
| Escada C→D | 2,5 m de largura, 20 degraus | portão P3 na base; parede lateral até 2200 |
| D — plataforma de saída | 6 × 14 m | porta de saída de 4 × 4,5 m na parede leste, com moldura e letreiro “SAÍDA” |

## 3. Planta de cada andar
- `00_plantas_diagrama.png`: diagrama com cotas dos 4 andares.
- `00_corte_diagrama.png`: corte com as alturas.
- Capturas reais da Unreal: `capturas/02..05_planta_*` (vista superior de cada andar) e `06_corte_lateral_alturas`.

## 4. Percurso previsto
1. Os dois começam no saguão (A). Uma apresentação inicial de cerca de 4,8 s mostra o poço e a porta de saída; pode ser pulada com **E**.
2. Os dois seguem pelo corredor do poço até o elevador.
3. **P1 (futuro):** a cabine está travada. A Ruína destrói a trava e o Neon reconstrói o suporte. Com o elevador destravado, os dois sobem juntos usando o console da cabine.
4. Em B, as portas laterais se abrem. A Ruína atravessa a barreira roxa e vai para a sala norte; o Neon atravessa a barreira ciano e vai para a sala sul.
5. **P2 (futuro):** com o mecanismo equilibrado, os portões P2 se abrem e os dois se reencontram a leste. Depois sobem a escada B→C.
6. Em C, ficam a faixa norte e a cobertura sul. **P3 (futuro):** a esfera ativa o sensor, o que abre o portão P3. Os dois sobem a escada C→D até a porta de saída.

## 5. Áreas reservadas (placeholders identificados por nome e cor; sem lógica)
- **P1:**
  - `P1_Trava_Destrutivel` (vermelha, face norte da casa de máquinas);
  - `P1_Suporte_Reconstruivel` (azul, face sul, sem colisão);
  - `P1_Mecanismo_Tambor`;
  - `P1_Contrapeso_Elevador`;
  - área de aproximação.
- **P2:**
  - 2 × `P2_PlataformaSuspensa` (com cabos e carga);
  - freios e trava de liberação na sala da Ruína;
  - guia reconstruível e suporte de transferência na sala do Neon;
  - `P2_Contrapeso_Central` no hub, visto pelas janelas das duas salas;
  - `P2_Rota_Reencontro`.
- **P3:**
  - lançador;
  - `P3_Trecho1/2/3_Construivel` (450/450/350 cm, em leve declive);
  - esfera;
  - sensor final;
  - `P3_Coletor_Seguranca`.
- **Portões:** `P2_Norte`, `P2_Sul` e `P3_Saida` (ator `CoopGate`, replicado, preparado para ser aberto pelos puzzles).
- **D:** porta de saída como placeholder, sem lógica.

## 6. Elementos integrados
**Elevador `ACoopElevator`** (o mesmo das Tarefas 03 e 04)
- **Novo modo explícito** (`bExplicitControl`): não parte sozinho com os personagens a bordo. Só parte por pedido feito no console e **validado no servidor**.
- **O servidor recusa o pedido** se o elevador estiver travado, em movimento, já no andar pedido, ou se houver alguém na soleira da porta.
- **Trava do Puzzle 1:** `bLockedP1` é replicado. O P1 futuro vai chamar `ServerSetLocked(false)`.
- **Mensagem ao jogador quando travado:** “Mecanismo TRAVADO (Puzzle 1 ainda não resolvido)”, para não parecer que o puzzle foi concluído.
- **Portas de patamar fixas** (`bLandingDoors`): fecham o poço em A e em B quando a cabine não está no andar.
- **Compatibilidade:** o modo antigo, automático, continua sendo o padrão do `Fase2_ElevTeste`. Foi testado de novo e continua igual.

**Outras peças**
- **`ACoopConsole` (novo):** console usado com a tecla **E** (`ServerInteract`). Há um na cabine (alterna o andar), um de chamada em A e um de chamada em cada sala de B.
- **`ACoopRoleBarrier`:** a mesma barreira das tarefas anteriores, sem mudanças.
- **`ACoopGate` (novo):** portão de progresso replicado.
- **`ACoopIntroCam` (nova):** apresentação inicial. A pose é calculada a partir da hora do servidor. Toca uma vez por sessão, quando os dois jogadores estão presentes, e não repete no reinício.
- **`CoopFase2GameMode`:** recursos ativados **só** em mapa que tenha o marcador `F2X_Layout`. Com isso, o `Fase2_Blockout` continua igual. Os recursos são:
  - **Recuperação de queda:** uma queda livre de mais de 3,5 m leva o jogador ao ponto seguro do nível de onde caiu (A, B por papel, C, D).
  - **Apresentação inicial:** controla a câmera da apresentação e devolve o controle no fim.
  - **E:** pula a apresentação ou usa o console.
  - **Depuração separada da progressão:** F6–F9 teletransportam entre os níveis. F10 abre os portões e destrava o elevador, com o aviso “[DEPURAÇÃO] NÃO é solução de puzzle”.

## 7. Higgsfield e Blender
- **Higgsfield 3D Jutsu: não foi usado na geometria.**
  - A conta está no plano gratuito com **0 créditos**.
  - O catálogo de modelos do 3D Jutsu não retornou nada para “industrial railing” e “pipe”.
  - Não comprei créditos nem gerei nada.
- **Blender (sem custo):** criei 6 módulos procedurais em `MapaFase2/Modulos/*.fbx` e `F2_Modulos.blend`, com escala real e pivô na base. Foram importados em `/Game/Fase2/Modulos`.
  - guarda-corpo de 200 cm (com colisão);
  - treliça de 400 cm;
  - painel de parede de 200 × 300 cm;
  - grade de piso de 200 × 200 cm;
  - tubo de 400 cm;
  - pilar em perfil I de 600 cm.
- **Unreal:** todos os guarda-corpos do mapa usam o módulo. Painéis, treliças, pilares, grades e tubos são só visuais (sem colisão).
- **Placeholders:** blocos e pisos continuam como cubos coloridos do blockout.

## 8. Testes (multiplayer local: host = Neon, cliente = Ruína, mesma máquina)
| Teste | Resultado |
|---|---|
| Apresentação inicial | completa no host (4,8 s); **pulada** com E no cliente (1,5 s); controle devolvido nos dois |
| Elevador com trava do P1 | pedido **recusado**, ficou parado (nos dois) |
| Embarque, subida A→B (acionamento explícito) | OK, os dois a bordo; desvio de altura ≤ 0,1 cm |
| Barreira própria | Ruína atravessou a roxa; Neon atravessou a ciano |
| Barreira do outro (andando e pulando) | **bloqueados** (\|Y\| máx. 186; barreira em 220–240) |
| Chamada/retorno | Neon desceu sozinho; Ruína chamou da sala norte, desceu, os dois subiram de novo |
| Validação da soleira | 1 pedido recusado (“alguém está na porta”), aceito depois |
| Portão P2 fechado (andando e pulando) | **bloqueado** (X máx. 1916; portão em 1950) |
| Reencontro (portões abertos por depuração) | os dois juntos, ~6 m de distância |
| Escada B→C e C→D | OK (Z 1290 e 1890) |
| Queda proposital no poço (pulando o guarda-corpo em C) | caiu até Z 90 e **voltou ao ponto seguro de C** |
| Regressão `Fase2_ElevTeste` (modo automático, 4 ciclos) | OK, igual à Tarefa 04 |
| Regressão `Fase2_Blockout` | OK (capturas e log iguais) |
| Regressão Fase 1 oficial (puzzle completo) | SUCESSO |

- **Dois computadores reais:** PENDENTE.
- **Saltos indevidos, verificados por geometria:**
  - não é possível subir 6 m pulando;
  - barreiras têm 3 m e portões 4 m de altura;
  - as salas são fechadas até o teto (6 m);
  - a escada C→D tem parede lateral até 2200.

## 9. Problemas e riscos
- **Guarda-corpo:** 110 cm é pulável (pulo de ~215 cm). A recuperação de queda resolve, mas usa teletransporte. Avaliem se querem guarda-corpos mais altos ou telas nas bordas do poço.
- **Câmera dentro da cabine:** a cabine de 2,5 m deixa a câmera de 3ª pessoa muito próxima dos personagens (veja o vídeo). Considerar uma cabine de 3 × 3 m ou um ajuste de câmera ao entrar.
- **Sem aviso de tecla:** não há indicação “E: usar” na tela perto dos consoles (pendência de interface).
- **Escadas como blocos maciços:** na escada C→D, a parte entre Y 150 e 250 fica suspensa sobre o reencontro.
- **Cobertura sul (P3):** fica acessível a qualquer momento por C. Não pula etapas, mas ainda é preciso definir como a esfera interage com a escada C→D e o sensor.
- **Iluminação** é provisória. A apresentação passa perto de uma parede no 2º trecho.
- **Vértice:** é bloqueado pelas duas barreiras. É o comportamento esperado para o modo Duo.
- **Elevador sem cabos:** os jogadores podem ficar em A e B ao mesmo tempo. Testado: não há soft lock, porque há consoles de chamada nos dois andares.

## 10. Arquivos
**Código novo**
- `CoopConsole.*`, `CoopGate.*`, `CoopIntroCam.*`
- `CoopF2Test.*` (testes automáticos `-F2Fluxo`, `-F2Vistas`, `-F2Intro`)

**Código alterado**
- `CoopElevator.*`: modo explícito, trava do P1, portas de patamar.
- `CoopCharacter.*`: tecla E e `ServerInteract`.
- `CoopFase2GameMode.*`: recursos do layout funcional.
- `CoopTestSubsystem.cpp`: cria os novos testes.

**Conteúdo**
- `Content/Maps/Fase2_Funcional.umap`
- `Content/Fase2/Modulos/*`

**Scripts**
- `MapaFase2/CriarFase2Funcional.py`
- `MapaFase2/Modulos/*`
- `F2FuncCriar.bat`, `F2FuncTestes.bat`, `F2FluxoSo.bat`, `F2IntroGravar.bat`, `F2Regressao.bat`

## 11. Git e reversão
Ver o commit “Fase 2 T05”. Para reverter: `git revert <commit>`.
- O `Fase2_Blockout` não depende de nada novo.
- Sem o mapa experimental, o GameMode volta ao comportamento anterior.
- O elevador fica no modo antigo por padrão.

## 12. Próxima tarefa recomendada (uma)
**Implementar o Puzzle 1 (Elevador de Emergência) no `Fase2_Funcional`:**
- tornar a trava destrutível pela Ruína e o suporte reconstruível pelo Neon, com a animação de destroços → outro objeto que já existe na Fase 1;
- destravar o elevador (`ServerSetLocked(false)`);
- incluir a dica “E: usar”;
- testar em 2 PCs reais.
