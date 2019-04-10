package client.controller;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Scene;
import javafx.scene.layout.BorderPane;
import javafx.stage.Stage;

public class ClientApp extends Application {
	
	//	登录界面的一些参数
	private final String title = "登录";
	private final String loginViewResource = "/client/view/LoginView.fxml";
	private Stage stage;
	private BorderPane loginView;

	@Override
	public void start(Stage primaryStage) {
		this.stage = primaryStage;
		this.stage.setTitle(this.title);
		
		showLoginView();
	}

	/**
	 * 展示登录界面
	 */
	private void showLoginView() {
		try {
			FXMLLoader loader = new FXMLLoader();
			loader.setLocation(ClientApp.class.getResource(this.loginViewResource));
			this.loginView = (BorderPane)loader.load();
			this.stage.setScene(new Scene(loginView));
			this.stage.setResizable(false);
			this.stage.show();
			
			//	获得登陆界面的控制器
			LoginViewController controller = loader.getController();
			controller.setClientApp(this);
		} catch (Exception e) {
			e.printStackTrace();
		}
		
	}

	public static void main(String[] args) {
		launch(args);
	}
}
