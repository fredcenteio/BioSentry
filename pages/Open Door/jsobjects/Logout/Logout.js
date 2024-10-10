export default {
	handleLogout(){
		storeValue('user', null);
		navigateTo('Login');
  }
	
};
