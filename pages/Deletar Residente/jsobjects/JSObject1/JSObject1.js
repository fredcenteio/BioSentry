export default {
 async myFun2 () {
		if(appsmith.store.user!="fredcenteio.adm@biosentry.com"){
		Button2.setDisabled(true);
		Button2.setVisibility(false);
		
		}else{
			Button2.setDisabled(false);
			Button2.setVisibility(true);
		}
	}
}