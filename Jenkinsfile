pipeline {
	agent any
	
	environment {
		DOCKERHUB_CREDENTIALS = credentials('DockerHub')	
	}

	stages {
		stage('docker build') {
			steps {
				script {
					sh "docker build -f Dockerfile -t rhiiitech/sockserver:1.0.0-${BUILD_ID} ."
				}
			}
		}
		stage('login dockerhub') {
			steps {
				script {
					sh 'echo $DOCKERHUB_CREDENTIALS_PSW | docker login -u $DOCKERHUB_CREDENTIALS_USR --password-stdin'
				}
			}
		}
		stage('docker push') {
			steps {
				script {
					sh "docker push rhiiitech/sockserver:1.0.0-${BUILD_ID}"
				}
			}
		}
	}
}
