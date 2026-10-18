# Fase 1 — como encaixar os modelos do Blender

O mapa `Fase1_Teste` e montado com blocos-placeholder. Cada peca tem um **slot** (nome). Quando existir o
modelo `/Game/Fase1/<Slot>/SM_<Slot>`, ele aparece no lugar do placeholder automaticamente (sem mexer em codigo).

## Passo a passo
1. No Blender: exporte `ImportFase1\<Slot>\SM_<Slot>.fbx` + texturas `T_<Slot>_basecolor/normal/roughness/metallic.png`
   (mesmo padrao do `Coluna_Inteira`). Escala real: 1 unidade Unreal = 1 cm. **Origem do modelo = centro da BASE.**
2. Feche o Editor e rode `ImportarFase1.bat` (importa para `/Game/Fase1/<Slot>/`).
3. Abra `TestarFase1.bat`. O placeholder some e o modelo entra. A pasta `/Game/Fase1` ja e empacotada (DefaultGame.ini).

## Regras
- **Colisao**: continua sendo a do bloco-placeholder (o modelo nao tem colisao propria). Se o modelo for maior/menor que o
  placeholder, ajuste o tamanho no `CoopFase1GameMode.cpp` (coluna `Size`/`FVector` do slot).
- Um modelo por slot: todas as pecas do mesmo slot (ex.: `Escombro`) usam o mesmo modelo, cada uma na sua posicao/rotacao.
- Slot sem modelo = continua o placeholder. Pode ir trocando aos poucos.

## Slots estruturais (placeholder: tamanho em cm, origem = centro do bloco)
| Slot | Placeholder | Obs |
|---|---|---|
| `Coluna_Inteira` | 160x160x560 | destrutivel; escombros = pedacos procedurais por enquanto |
| `Ponte_Inteira` | 1000x250x30 | destrutivel; nasce destruida; topo em Z=+40 |
| `Ponte_Quebrada` | 150x240x24 | toco fixo ao lado da saida |
| `Plataforma_Inicial` | 1400x1000x600 | topo em Z=0 |
| `Plataforma_Saida` | 1400x1000x640 | topo em Z=+40 |
| `Lava` | 1000x1000x100 | fundo do vao, topo em Z=-350 |
| `Parede_Lateral_Esq` / `_Dir` | 3850x40x1300 | |
| `Parede_Fundo_Inicio` / `_Saida` | 40x1080x1300 | |
| `Teto_Principal` | 3890x1080x40 | cai no colapso |
| `Teto_Estrutural` | 600x1000x60 | sobre a coluna; cai no colapso |
| `Porta_Saida` | 10x200x260 | |

## Slots de decoracao (sem colisao)
`Parede_Rodape`, `Parede_Junta`, `Neon_Parede`, `Janela_Moldura`, `Janela_Vidro`, `Cano_Teto`, `Viga_Quebrada`, `Lava_Gota`,
`Lava_Fluxo`, `Fogo`, `Escombro`, `Piso_Junta`, `Faixa_Perigo`, `Disco_Piso`, `Corrimao`, `Corrimao_Poste`, `Placa_Saida`,
`Porta_Moldura`, `Mezanino`, `Mezanino_Corrimao`, `Mezanino_Pilar`.
