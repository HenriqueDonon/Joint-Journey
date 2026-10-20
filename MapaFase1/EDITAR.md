# Editar a Fase 1 no Editor da Unreal

1. Feche o jogo. Rode `EditarFase1.bat` (abre o Editor direto no mapa `Fase1_Editavel`).
2. No Organizador (Outliner): pasta **Decor** = modelos que voce pode mover/girar/duplicar (Alt+arrastar)/apagar.
   Pasta **Referencia** = plataformas, paredes, teto, porta, coluna, ponte (so aparecem no Editor; mexer nelas NAO muda o jogo).
   Dica: clique no olho da pasta `Referencia/Teto` para esconder o teto enquanto edita.
3. W mover, E girar, R escalar, botao direito + WASD para voar, F foca na peca selecionada. Para pegar modelo novo: arraste de `Content/Fase1/<Modelo>/SM_<Modelo>`.
4. Salve com Ctrl+S. Teste com `TestarFase1.bat` (ele usa o Fase1_Editavel automaticamente).
5. Para os amigos: `ProntoParaAmigo.bat` (empacota ja com o mapa editado).

Peca com colisao: no painel Detalhes > Colisao > Predefinicoes = BlockAll. Decoracao sem colisao = NoCollision.
Refazer o mapa do zero a partir do codigo (APAGA suas edicoes): crie `MapaFase1\RECRIAR.txt` e rode `CriarFase1Editavel.bat`.
