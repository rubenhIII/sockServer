pipeline {
	agent any

	stages {
		stage('docker build') {
			steps {
				script {
					sh "docker build -f Dockerfile -t rhiiitech/sockserver:1.0.0-${BUILD_ID} ."
				}
			}
		}
		stage('docker push') {
			steps {
				script {
					sh "docker push rhiiitech/sockserver:1.0.0-${BUILD-ID}"
				}
			}
		}
	}
}
