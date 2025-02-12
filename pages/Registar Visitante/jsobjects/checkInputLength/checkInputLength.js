export default {
  myVar1: [],
  myVar2: {},

  // Função para verificar o comprimento do texto em todas as entradas
  checkInputLength() {
    // Verifica se todos os campos de entrada têm comprimento maior que 0
    if (
      InputNome.text.length > 0 &&
      InputApelido.text.length > 0 &&
      InputCodigo.text.length > 0 &&
      InputStatus.text.length > 0
    ) {
      ButtonRegistarV.setVisibility(true); 
      ButtonRegistarV.setDisabled(false); // Habilita o botão se todos os campos estiverem preenchidos
    } else {
      ButtonRegistarV.setVisibility(false); 
      ButtonRegistarV.setDisabled(true); // Desabilita o botão se algum campo estiver vazio
    }
  }
}