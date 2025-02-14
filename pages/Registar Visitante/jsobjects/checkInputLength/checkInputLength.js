export default {
  myVar1: [],
  myVar2: {},

  // Função para verificar o comprimento do texto em todas as entradas
  checkInputLength() {
    if (
      InputNome.text.length > 0 &&
      InputApelido.text.length > 0 &&
      InputCodigo.text.length > 0 &&
      InputStatus.text.length > 0 &&
      SelectEdificio.selectedOptionLabel
    ) {
      ButtonRegistarV.setVisibility(true); 
      ButtonRegistarV.setDisabled(false);
    } else {
      ButtonRegistarV.setVisibility(false); 
      ButtonRegistarV.setDisabled(true);
    }
  }
}