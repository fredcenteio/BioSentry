export default {
	myVar1: [],
	myVar2: {},
	
	// Função para realizar a query
	async myFun1 () {
		// Executa a query (exemplo com debug.data)
		this.myVar1 = debug.data;  // Substitua isso pela sua consulta real
		console.log("Query executada", this.myVar1);
	},

	// Função para recarregar a query automaticamente a cada X milissegundos
	async myFun2 () {
		const intervalTime = 2; // Defina o intervalo em milissegundos (5000 = 5 segundos)
		
		setInterval(async () => {
			await this.myFun1();  // Chama a função que executa a query
			
		}, intervalTime);
		return debug.data;
	}
	
}
