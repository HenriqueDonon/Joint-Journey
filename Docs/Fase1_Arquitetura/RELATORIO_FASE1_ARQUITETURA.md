# Joint Journey | Fase 1 | Reconstrução arquitetônica experimental (cenário Higgsfield)

Status: **mapa experimental jogável. A sequência cooperativa completa, o colapso do teto e a ausência de atalho foram confirmados por teste local** (host + cliente na mesma máquina). **Teste em 2 computadores: PENDENTE.** Nenhum mapa oficial foi substituído.

## 1. Mapa
`/Game/Maps/Fase1_Arquitetura` → `Content/Maps/Fase1_Arquitetura.umap`. É gerado por `MapaFase1Arq/CriarFase1Arquitetura.py` via `CriarFase1Arquitetura.bat`.
Mapas preservados (SHA-1 conferido):
- `Fase1_Editavel` (oficial): `f3b9c861…`
- `Fase1_Experimental`: `cd3b3e2f…`, igual ao commit `12fb567`
- `Fase2_Blockout`: `c4b76531…`

## 2. Abordagem: migração (opção A), com integração por marcadores
**Arquivos de origem:** o projeto 3D Jutsu `bb4aa9f9…` na revisão 10, baixado sem custo (`Higgsfield_Fase1/jj_fase1_higgsfield_rev10.blend|.glb`).

**Edição no Blender local** (gerou `jj_fase1_layout2.blend|.glb`; o arquivo original foi preservado). Removi o que permitia ignorar o puzzle ou o que é substituído por atores de gameplay:
- **Rota de desvio:** rampa 1, plataforma suspensa 1, rampa 2, cabos e corrimãos. Ela atravessava o vão sem a ponte.
- **Plataforma 2 do mirante:** ficava a 7 m da borda inicial, ao alcance de um pulo. O píer foi encurtado para 10 m de vão e ganhou uma guarda na ponta.
- **Peças no meio do vão que viravam degraus:** os restos da ponte quebrada, as barreiras, um gerador e um console sobre as plataformas removidas.
- **Substituídos por atores de gameplay:** a coluna monumental e a ponte do Higgsfield (no lugar delas entram os atores destrutíveis da Fase 1, com modelos do Blender, fragmentos e replicação).
- **Folhas da porta:** a porta funcional da Fase 1 entra na moldura do Higgsfield.

**Importação na UE:** o glTF virou 32 malhas com seus materiais PBR, com colisão "complexa como simples" (exceto brasas, faixas de piso e emblema).

## 3. Elementos aproveitados do Higgsfield
- **Arquitetura e circulação:** o galpão de 46×30×36 m, as plataformas, o vão de 16×28×16 m com metal fundido e destroços, e as brasas.
- **Estruturas:** treliças do teto, tubulações, pilastras, faixas de perigo, rampa e púlpito da saída, moldura e emblema da porta, o píer do mirante e os corrimãos (ajustados).
- **Props do catálogo:** consoles, geradores, transformadores, barris, caixas e sinalizadores.
- **Iluminação:** 18 luzes refeitas como luzes nativas, nas mesmas posições e cores. São 7 refletores frios (com sombra), 6 focos de fogo no vão, o ciano da porta, as faíscas e os sinalizadores.

## 4. Modelos reutilizados do Blender (Fase 1) e da Unreal
- **Gameplay (sem alteração):**
  - `Coluna_Inteira`, com seus 10 fragmentos;
  - `Ponte_Inteira`;
  - porta `Porta_Quadro` e `Porta_Folha_*`;
  - o túnel de transição completo (placas, painéis, estruturas e portal);
  - o teto que desaba (`Teto_Painel`, agora como deck estrutural apoiado na coluna).
- **Ambientação:** `Pilha_Escombro` ×3, `Viga_Quebrada` ×2, `Estrutura_Aco` ×2, `Cabos`, `Pilar_Ruido` e `Fachada_Queimada`. Ficam fora do percurso; os de chão não têm colisão, para não criar degraus.
- **Unreal:** pós-processamento (exposição −1, vinheta, contraste, temperatura fria), além do sol, céu e névoa do código.

## 5. Alterações de código (compatíveis com os mapas anteriores)
- `CoopFase1GameMode` ganhou o **"layout 2"**, ativado só quando o mapa tem os marcadores `F1L_Coluna`, `F1L_Ponte` e `F1L_Porta`. Sem eles, roda o layout original, inalterado.
- **No layout 2, o código cria só os sistemas de gameplay**, nas posições dos marcadores:
  - coluna e ponte, com material compartilhado, comprimento pelo marcador, replicação e fragmentos;
  - teto estrutural que desaba (mesma queda, esmagamento e limite de 12 s);
  - rachaduras, que aparecem na parede indicada pelo marcador;
  - porta e túnel completos, deslocados para a moldura do Higgsfield;
  - barreira provisória do vão, pontos de início e névoa.
- **Refatorações mínimas:** o trecho da porta virou a função `BuildExitDoor(FX,FY,FZ)`, chamada com os mesmos valores no layout original. A barreira e a zona de conclusão aceitam deslocamento; o deslocamento padrão é zero.
- **Poderes, regras e condições de conclusão não foram alterados.**
- `CoopTestSubsystem` (só teste, inativo sem parâmetros):
  - o roteiro do puzzle lê os marcadores;
  - novas variantes `-F1Colapso` e `-F1Atalho`;
  - tour de câmera do novo layout.

## 6. Testes (locais: host = Neon, cliente = Ruína, mesma máquina)
| Teste | Resultado |
|---|---|
| Sequência completa: Ruína destrói a coluna → Neon constrói a ponte → atravessam → Ruína destrói a ponte → Neon reconstrói a coluna → porta → fim do túnel | **SUCESSO**, 2 execuções (coluna fora por 3,8 s, abaixo do limite) |
| Ações pelo caminho real do jogo (RPC `ServerPowerShot`: papel, alcance, linha de visão) | ok |
| Colapso: coluna destruída e não reconstruída → teto desaba → fase reinicia (coluna inteira, ponte destruída) | **SUCESSO** |
| Queda no vão → volta ao início | ok (observada várias vezes no log do teste de colapso) |
| Atalho: 3 pulos com embalo saindo da borda (centro, píer, diagonal) | **SEM ATALHO**: alcance máximo X=1951 (lado de saída em 2800, píer em 2200); a barreira provisória também barra |
| Regressão: mesma sequência no `Fase1_Editavel` (layout original) com o código novo | **SUCESSO** |

Evidências: `video_puzzle_host_cliente_2x.mp4`, `video_colapso_teto_2x.mp4`, `video_exploracao_camera.mp4` (30 fps, gravado na UE), `comparativo_original_higgsfield_nova.jpg`, `capturas_nova_arquitetura_9vistas.jpg`, `quadros_puzzle_host_cliente.jpg` e os logs `log_*`.

## 7. Problemas e pendências
- **Pendente:** teste em 2 PCs; medição de desempenho (FPS); exploração livre por pessoa (só o roteiro automático foi testado).
- **Visual provisório:** os materiais do Higgsfield são cores lisas, sem textura. As superfícies grandes de concreto (rampa da saída, faces do vão) ficam claras sob os refletores. Fogo, faíscas e fumaça ainda não têm partículas (Niagara): hoje são brasas estáticas, metal fundido emissivo e luzes.
- **O deck estrutural** (o teto que desaba) é visível de cima e tem acabamento simples.
- **Escala:** a porta funcional da Fase 1 (1,3 m de vão) fica pequena dentro da moldura monumental do Higgsfield (7×8 m). Ela foi completada com paredes do código.
- **Detecção de atalho:** é automática e limitada a 3 trajetórias. Outros pontos (pilastras, tubos, píer) foram verificados só por geometria.
- **Importação:** a importação glTF espelha o eixo Y; o script detecta e compensa.

## 8. Git / reversão
Ver commit desta tarefa. Reverter: `git revert <commit>`.
- O código do layout 2 só age com marcadores.
- Também dá para apagar `Content/Maps/Fase1_Arquitetura.umap` e `Content/Fase1Arq/`.
- Os arquivos de origem do Higgsfield ficam em `Higgsfield_Fase1/`.
