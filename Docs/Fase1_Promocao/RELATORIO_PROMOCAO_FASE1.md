# Joint Journey | Fase 1 | Correções finais e promoção do mapa oficial

Autor: Henrique Collinetti, com Claude. Data: 30/10/2026.

**Status**
- **Mapa oficial:** a nova arquitetura é agora a Fase 1 oficial (`/Game/Maps/Fase1`).
- **Testes:** todos passaram com host e cliente na mesma máquina, antes e depois da promoção.
- **Teste em dois computadores reais:** **PENDENTE**. A promoção não é homologação do multiplayer.

## 1. Correções
**Detritos vibrando**
- **Causa:** os detritos pequenos do colapso (`ACoopCollapseRig`) continuavam girando depois de tocar o piso, porque a rotação era calculada pelo tempo total desde o lançamento e não parava no pouso. Isso acontecia na instabilidade e depois da estabilização.
- **Correção:** a rotação agora é limitada ao instante do pouso. Os detritos que já caíram continuam no chão e ficam parados.
- **O que não mudou:** os efeitos temporários (poeira, faíscas, luzes) já encerravam na estabilização. A transferência de material, o colapso e o reset estão intactos.
- **Medição** (log `DETRITOS` em cada quadro, nas duas máquinas, depois da reconstrução):
  - 26–27 de cada ~30 amostras têm movimento 0,000.
  - Os únicos picos são de detritos que ainda estavam em queda quando a coluna foi reconstruída: aparecem só no instante do pouso e depois ficam em 0.

**Colisão da ponte**
- **Antes:** a ponte ficava sólida assim que o Neon a reconstruía, ~1,5 s antes de a montagem visual terminar.
- **Agora:** a ponte só fica sólida quando a montagem visual termina (~1,52 s), com a mesma regra no servidor e nos clientes.
- **Como foi feito:** o servidor registra a hora de liberação (`SolidAt`, hora do servidor, replicada) e cada máquina liga a colisão nesse instante.
- **Limites:** a coluna não mudou e continua sólida na hora, porque a reconstrução dela interrompe o colapso. O limite de 12 s e a duração da animação não mudaram.
- **Medição:**
  - aos 0,5–0,6 s, colisão = NÃO (nas duas máquinas);
  - sólida em 1,32–1,71 s, conforme o momento em que cada máquina detecta a reconstrução (esperado ~1,52 s).

## 2. Testes antes da promoção (`Fase1_Arquitetura`, host + cliente locais)
| Teste | Resultado |
|---|---|
| Destruição da coluna, construção e travessia da ponte, destruição da ponte, reconstrução da coluna, conclusão | SUCESSO (2/2 no fim do túnel), em 2 execuções |
| Ponte sem colisão durante a montagem | OK |
| Interrupção da instabilidade (8,02 s e 11,71 s, limite de 12 s) | OK, nenhum esmagamento |
| Destroços estabilizados | OK |
| Derrota, desabamento e reset | SUCESSO nas duas máquinas |
| Atalho (3 tentativas de pulo pelo vão) | SEM ATALHO (X máximo 1951; o lado de saída começa em 2800) |
| Mapa oficial antigo (regressão) | SUCESSO |

## 3. Backup (antes de qualquer substituição)
- **Estado do Git:** `main` em dia com `origin`.
- **Ponto de recuperação no Git:** commit `e644960` com a tag **`fase1-original-backup`** (enviada ao GitHub). Ali o `Fase1_Editavel` ainda é o mapa oficial.
- **Atenção, encontrado na inspeção:** a pasta `Content/Fase1` (~200 MB de modelos e materiais usados pelas duas versões da Fase 1) **não estava versionada no Git**, assim como `Fase1_Teste.umap`. Por isso foi feita uma cópia completa local: **`Backups/Fase1_Original_2026-10-29/Fase1_Original_completo.zip`** (204 MB, 295 arquivos). Contém:
  - `Content/Fase1` inteiro;
  - os mapas `Fase1_Editavel`, `Fase1_Teste` e `Fase1_Experimental`;
  - `MapaFase1`, `Config` e os .bat de lançamento.
- **Cópia jogável dentro do projeto:** `/Game/Maps/Backup/Fase1_Original_Editavel` (testada: SUCESSO).
- **Proteção extra:** os assets de `Content/Fase1` foram enviados ao Git em um commit separado (ver Git).

## 4. Promoção
- **Como a Fase 1 é iniciada:** pelo `HospedarFase1.bat` (abre o mapa direto com `?listen`) e pela lista de empacotamento (`Empacotar*.bat` e `MapsToCook`). O menu principal leva à `Sala1`, sem referência à Fase 1.
- **O que referencia a Fase 1:** o `CoopFase1GameMode` é definido nas configurações de cada mapa. O código não referencia o nome do mapa; o layout 2 é ativado pelos marcadores `F1L_*`. A saída faz `ServerTravel` para `/Game/Maps/Fase2`, sem mudança.
- **Substituição** (pelo editor, script `MapaFase1Arq/Promover.py` / `PromoverFase1.bat`):
  - `/Game/Maps/Fase1_Editavel` → `/Game/Maps/Backup/Fase1_Original_Editavel`;
  - `/Game/Maps/Fase1_Arquitetura` → **`/Game/Maps/Fase1` (OFICIAL)**.
  - O renomear direto do editor abriu uma caixa de diálogo (por haver nomes em textos) que é cancelada no modo automático. Por isso os mapas foram duplicados pelo editor e o nome antigo foi removido. O efeito é o de um renomear; a cópia anterior está na tag e no zip.
- **Verificação dos mapas:**
  - `Fase1`: 71 atores, `CoopFase1GameMode`, rig e revestimento do túnel presentes, 8 marcadores.
  - Backup: 746 atores, `CoopFase1GameMode`.
- **Atualizados:**
  - `Config/DefaultGame.ini`: `MapsToCook` com `Fase1` e o backup; `DirectoriesToAlwaysCook` com `/Game/Fase1Arq` e `/Game/Fase2/Materiais`, que o código carrega em tempo de execução e que o empacotamento poderia deixar de fora.
  - `HospedarFase1.bat`, `EditarFase1.bat` e `Empacotar*.bat` → `Fase1`.
  - `EstadoAtual.bat`, `ColunaShot.bat` e `ColunaVideo.bat` (ferramentas do mapa antigo) → backup.
  - Scripts geradores apontam para `/Game/Maps/Fase1`.
  - Comentários no código foram atualizados, sem mudança de lógica.
- **Mapas não oficiais que continuam como estavam:** `Fase1_Experimental` (versão visual experimental) e `Fase1_Teste` (base antiga de geração). Nenhum é carregado por padrão.

## 5. Testes depois da promoção (abrindo `/Game/Maps/Fase1?listen`, o mesmo caminho do `HospedarFase1`)
| Teste | Resultado |
|---|---|
| Nova arquitetura carregada (`LoadMap /Game/Maps/Fase1`, layout 2 ativo) | OK |
| Personagens aparecem, poderes funcionam, puzzle completo e saída pelo túnel | SUCESSO (2/2), em 2 execuções |
| Ponte só sólida após a montagem | OK (0,55–0,61 s: não; sólida em 1,49–1,71 s) |
| Salvamento (8,02 s e 11,63 s) e detritos parados | OK |
| Derrota, cena de desabamento e reset | SUCESSO nas duas máquinas |
| Referências quebradas | nenhuma (o log só mostra DLLs de profiler ausentes, normal) |
| Backup `Backup/Fase1_Original_Editavel` | SUCESSO (2/2) |
| Fase 2 intacta | SHA igual: `Fase2_Blockout` `c4b76531`, `Fase2_ElevTeste` `96e8bb08`; `Fase1_Experimental` `cd3b3e2f` |

## Vídeo
**`V_Fase1_Oficial.mp4`** (64 s, gravado na Unreal), em 4 partes:
1. A nova Fase 1 oficial (passeio a 30 fps).
2. A ponte termina a montagem antes da travessia.
3. A reconstrução da coluna interrompe a instabilidade e os detritos ficam parados.
4. Puzzle completo, host e cliente lado a lado.

As partes 2–4 são gravações de jogo a ~4–8 fps, em velocidade real.

## Problemas encontrados
- O renomear direto de mapa pelo editor não funciona no modo automático; foi resolvido com duplicar e remover.
- **Compilação final bloqueada:** um processo antigo do jogo ("Joint Journey", PID 34092, *Not Responding*) deixou o Live Coding ativo e bloqueou a compilação final. A última compilação válida é a do commit `e644960`; depois dela só mudaram comentários. Os testes rodaram com esse binário. Recomenda-se fechar esse processo e compilar uma vez.
- O jogo empacotado (`Build/`) não foi regerado nesta tarefa. O `HospedarFase1.bat` só abre a nova Fase 1 depois do próximo `Empacotar.bat`.

## Pendências
- Teste em dois PCs reais.
- Reempacotar o jogo.
- Decidir se o backup em `Backups/` (fora do Git, 204 MB) deve ir para armazenamento externo.

## Reversão
- **Pelo editor:** `ReverterPromocaoFase1.bat` volta os nomes dos dois mapas. Depois, `git revert` do commit da promoção restaura configuração e .bat.
- **Pelo Git:** `git checkout fase1-original-backup -- Content/Maps/Fase1_Editavel.umap Content/Maps/Fase1_Arquitetura.umap Config HospedarFase1.bat Empacotar.bat EmpacotarRapido.bat`.
- **Arquivos sem Git:** restaurar a partir de `Backups/Fase1_Original_2026-10-29/Fase1_Original_completo.zip`.
