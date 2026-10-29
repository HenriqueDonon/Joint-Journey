# Joint Journey | Fase 1 | Versão experimental visual (Unreal + Blender + Higgsfield)

Status: **cópia experimental criada e testada localmente** (host + cliente na mesma máquina). Mapa oficial intacto. Aguarda avaliação antes de qualquer integração.

## Mapa
- Experimental: `/Game/Maps/Fase1_Experimental` → `Content/Maps/Fase1_Experimental.umap` (gerado por `MapaFase1X/CriarFase1Experimental.py` via `CriarFase1Experimental.bat`).
- Oficial `/Game/Maps/Fase1_Editavel`: SHA-1 inalterado (`f3b9c861…`). Fase 2 inalterada (`Fase2_Blockout` `c4b76531…`).

## Como a cópia funciona (por que o gameplay fica preservado)
A jogabilidade da Fase 1 (plataformas, coluna, ponte, colapso do teto, porta, túnel, spawn) é criada pelo **código** (`CoopFase1GameMode`) igual em qualquer mapa; o mapa só guarda a decoração editável (`F1Decor`). A cópia acrescenta somente decoração/iluminação (tags `F1Decor` + `F1Exp`, pasta `Experimental/`). Nenhum ator interativo foi substituído; código de gameplay não foi alterado.

## Origem dos recursos
**Higgsfield (projeto 3D Jutsu `bb4aa9f9…`, revisão 10, baixado sem custo: `Higgsfield_Fase1/*.glb|.blend`):**
- Aproveitados: 5 props do catálogo, ou seja, gerador, console, cabine de transformador, pilha de barris e sinalizador de alarme (152–1.320 tri, cor por vértice, sem UV). Foram para `/Game/Fase1X/HF/*` com material `M_F1X_CorVertice`. São 9 instâncias, só junto às paredes e fora do percurso.
- Aproveitado como **ideia** (recriado nativamente na UE): o esquema de luz. São brilho quente vindo da lava, acento ciano na porta de saída e sinalizadores vermelhos.
- **Não aproveitados (justificativa):**
  - A arquitetura (galpão, plataformas, tubos e treliças): são malhas únicas mescladas, feitas para outro layout (44×28 m, vão de 16 m e percurso diferente). Usá-las exigiria adaptar o gameplay à decoração.
  - A caixa de suprimentos: é inferior à `Caixa_Madeira` já existente.
  - A porta blindada: a porta de saída existente é melhor e funcional.

**Blender (modelos já existentes no projeto):** todos os 35 modelos da Fase 1 continuam (já estavam em uso). Reaproveitei `Placa_Industrial` para revestir as faces do vão de lava: 44 placas, nas faces das plataformas e nas paredes laterais abaixo do piso, que antes eram superfícies lisas e claras.

**Unreal (nativo):** 10 luzes pontuais sem sombra (6 de lava, 1 ciano na porta, 2 vermelhas nos sinalizadores) e 1 PostProcessVolume. O pós-processamento faz:
- exposição −1,0, porque a cena estava estourada;
- vinheta 0,45, bloom 0,45 e AO 0,7;
- contraste ×1,08 e saturação leve.

## Testes (locais, mesma máquina; NÃO é teste em 2 PCs)
| Teste | Original | Experimental |
|---|---|---|
| Puzzle completo host (Neon) + cliente (Ruína): Ruína destrói coluna → Neon constrói ponte → ambos atravessam → Ruína destrói ponte → Neon reconstrói coluna (dentro dos 8 s do colapso) → porta → fim do túnel | SUCESSO | SUCESSO |
| Ações pelo caminho real do jogo (RPC `ServerPowerShot`: papel, alcance, linha de visão) | ok | ok (decoração nova não bloqueou mira) |
| 9 vistas fixas (`-AutoShot`) | capturado | capturado |
| Passeio de câmera 18 s, 30 fps fixos (`-F1Tour`) | gravado | gravado |

Tempos do puzzle (experimental): coluna destruída → coluna reconstruída em 3,8 s, sob o limite de 8 s; ambos chegaram ao fim do túnel.

**Pendente / não testado:**
- Teste em 2 PCs.
- Desempenho medido em quadros por segundo: não medi. São +10 luzes sem sombra, +44 malhas simples e +9 props low-poly, impacto esperado baixo.
- Mensagem "FASE 1 CONCLUÍDA" vista em imagem: a condição (ambos no fim do túnel) foi atingida, mas a mensagem não foi capturada.
- Colisão dos props testada só por inspeção: estão encostados nas paredes, longe do vão, sem acesso a atalhos.

## Problemas e limites
- Os placeholders criados pelo código continuam iguais ao oficial. São a laje branca caída na lava, os blocos de fogo e o teto de painéis muito reflexivo ("cromado"). Mudá-los exige alterar o código da Fase 1, o que não estava autorizado.
- Os props do Higgsfield são low-poly, com cor lisa. De perto contrastam com os modelos texturizados, por isso ficaram só no fundo.
- O `duplicate_asset` do editor derrubou o editor (o mapa oficial não foi afetado). A cópia foi feita com "Salvar como" (`save_map`).

## Viabilidade para as próximas fases
Viável e barato quando segue o mesmo padrão:
1. Gameplay no código, decoração no mapa.
2. Cópia do mapa para experimentar.
3. Iluminação e pós-processamento nativos (maior ganho visual).
4. Props avulsos de catálogo só como fundo.

Arquitetura gerada no Higgsfield/3D Jutsu só vale se for modelada **sobre a planta real da fase** e em peças separadas. Cenas mescladas de outro layout não servem.

## Git / reversão
Ver commit desta tarefa. Reverter: `git revert <commit>` ou apagar `Content/Maps/Fase1_Experimental.umap`, `Content/Fase1X/`, `Source/JogoCoop/CoopTestSubsystem.*` (só teste; inativo sem `-F1Tour`/`-F1Fluxo`).
