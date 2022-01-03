#include <cstdlib>
#include <random>
#include <iostream>
#include <fstream>
#include "contexte.hpp"
#include "individu.hpp"
#include "graphisme/src/SDL2/sdl2.hpp"
#include <chrono>
#include <mpi.h>

void màjStatistique( épidémie::Grille& grille, std::vector<épidémie::Individu> const& individus )
{
    for ( auto& statistique : grille.getStatistiques() )
    {
        statistique.nombre_contaminant_grippé_et_contaminé_par_agent = 0;
        statistique.nombre_contaminant_seulement_contaminé_par_agent = 0;
        statistique.nombre_contaminant_seulement_grippé              = 0;
    }
    auto [largeur,hauteur] = grille.dimension();
    auto& statistiques = grille.getStatistiques();
    for ( auto const& personne : individus )
    {
        auto pos = personne.position();

        std::size_t index = pos.x + pos.y * largeur;
        if (personne.aGrippeContagieuse() )
        {
            if (personne.aAgentPathogèneContagieux())
            {
                statistiques[index].nombre_contaminant_grippé_et_contaminé_par_agent += 1;
            }
            else 
            {
                statistiques[index].nombre_contaminant_seulement_grippé += 1;
            }
        }
        else
        {
            if (personne.aAgentPathogèneContagieux())
            {
                statistiques[index].nombre_contaminant_seulement_contaminé_par_agent += 1;
            }
        }
    }
}

void afficheSimulation(sdl2::window& écran, int largeur_grille, int hauteur_grille,
std::vector<épidémie::Grille::StatistiqueParCase>& statistiques, std::size_t jour)
{
    auto [largeur_écran,hauteur_écran] = écran.dimensions();
    //auto [largeur_grille,hauteur_grille] = grille.dimension();
    //auto const& statistiques = grille.getStatistiques();
    sdl2::font fonte_texte("./graphisme/src/data/Lato-Thin.ttf", 18);
    écran.cls({0x00,0x00,0x00});
    // Affichage de la grille :
    std::uint16_t stepX = largeur_écran/largeur_grille;
    unsigned short stepY = (hauteur_écran-50)/hauteur_grille;
    double factor = 255./15.;

    for ( unsigned short i = 0; i < largeur_grille; ++i )
    {
        for (unsigned short j = 0; j < hauteur_grille; ++j )
        {
            auto const& stat = statistiques[i+j*largeur_grille];
            int valueGrippe = stat.nombre_contaminant_grippé_et_contaminé_par_agent+stat.nombre_contaminant_seulement_grippé;
            int valueAgent  = stat.nombre_contaminant_grippé_et_contaminé_par_agent+stat.nombre_contaminant_seulement_contaminé_par_agent;
            std::uint16_t origx = i*stepX;
            std::uint16_t origy = j*stepY;
            std::uint8_t red = valueGrippe > 0 ? 127+std::uint8_t(std::min(128., 0.5*factor*valueGrippe)) : 0;
            std::uint8_t green = std::uint8_t(std::min(255., factor*valueAgent));
            std::uint8_t blue= std::uint8_t(std::min(255., factor*valueAgent ));
            écran << sdl2::rectangle({origx,origy}, {stepX,stepY}, {red, green,blue}, true);
        }
    }

    écran << sdl2::texte("Carte population grippée", fonte_texte, écran, {0xFF,0xFF,0xFF,0xFF}).at(largeur_écran/2, hauteur_écran-20);
    écran << sdl2::texte(std::string("Jour : ") + std::to_string(jour), fonte_texte, écran, {0xFF,0xFF,0xFF,0xFF}).at(0,hauteur_écran-20);
    écran << sdl2::flush;
}

void simulation(bool affiche, MPI_Comm comm)
{

    int rank;
    MPI_Comm_rank(comm, &rank);

    MPI_Datatype MPI_StatParCase;
    MPI_Type_contiguous(3, MPI_INT, &MPI_StatParCase);
    MPI_Type_commit(&MPI_StatParCase);

    MPI_Status status;

    int go; //indique qu'on attend un message

    int tag = 1234;

    bool quitting = false;



    //###############################
    //affichage par le processus 0
    //###############################

    if (rank==0)
    {
        constexpr const unsigned int largeur_écran = 1280, hauteur_écran = 720;
        sdl2::window écran("Simulation épidémie de grippe", {largeur_écran,hauteur_écran});

        sdl2::event_queue queue;


        std::size_t jours_écoulés;

        

        while(!quitting)
        {
            std::chrono::time_point<std::chrono::system_clock> debut_aff, fin_aff;
            auto events = queue.pull_events();
            for ( const auto& e : events)
            {
                if (e->kind_of_event() == sdl2::event::quit)
                    quitting = true;
            }
            debut_aff = std::chrono::system_clock::now();

            //données à recevoir et envoyer avec l'autre processus pour faire l'affichage
            int largeur_grille;
            int hauteur_grille;
            int s;

            
            MPI_Send(&go, 1, MPI_INT, 1, tag, comm); //indique au proc 1 qu'il attend un message

            MPI_Recv(&s, 1, MPI_INT, 1, 0, comm, &status);
            MPI_Recv(&largeur_grille, 1, MPI_INT, 1, 1, comm, &status);
            MPI_Recv(&hauteur_grille, 1, MPI_INT, 1, 2, comm, &status);
            std::vector<épidémie::Grille::StatistiqueParCase> statistiques;
            statistiques.reserve(s);
            MPI_Recv(statistiques.data(), 3*s, MPI_StatParCase, 1, 3, comm, &status);
            MPI_Recv(&jours_écoulés, 1, MPI_INT, 1, 4, comm, &status);



            if (affiche) afficheSimulation(écran, largeur_grille, hauteur_grille, statistiques, jours_écoulés);


            fin_aff = std::chrono::system_clock::now();
            std::chrono::duration<double> temps_aff = fin_aff - debut_aff;

            std::cout << "Temps affichage : " << temps_aff.count() << std::endl;
        
        }
    }


    //###############################
    //simulation par le processus 1
    //###############################

    if (rank==1)
    {
        unsigned int graine_aléatoire = 1;
        std::uniform_real_distribution<double> porteur_pathogène(0.,1.);


        épidémie::ContexteGlobal contexte;
        // contexte.déplacement_maximal = 1; <= Si on veut moins de brassage
        // contexte.taux_population = 400'000;
        //contexte.taux_population = 1'000;
        contexte.interactions.β = 60.;
        std::vector<épidémie::Individu> population;
        population.reserve(contexte.taux_population);
        épidémie::Grille grille{contexte.taux_population};

        auto [largeur_grille,hauteur_grille] = grille.dimension();
        // L'agent pathogène n'évolue pas et reste donc constant...
        épidémie::AgentPathogène agent(graine_aléatoire++);
        // Initialisation de la population initiale :
        for (std::size_t i = 0; i < contexte.taux_population; ++i )
        {
            std::default_random_engine motor(100*(i+1));
            population.emplace_back(graine_aléatoire++, contexte.espérance_de_vie, contexte.déplacement_maximal);
            population.back().setPosition(largeur_grille, hauteur_grille);
            if (porteur_pathogène(motor) < 0.2)
            {
                population.back().estContaminé(agent);   
            }
        }

        std::size_t jours_écoulés = 0;
        int jour_apparition_grippe = 0;
        int nombre_immunisés_grippe= (contexte.taux_population*23)/100;
        


        std::ofstream output("Courbe.dat");
        output << "# jours_écoulés \t nombreTotalContaminésGrippe \t nombreTotalContaminésAgentPathogène()" << std::endl;

        épidémie::Grippe grippe(0);

        

        std::cout << "Début boucle épidémie" << std::endl << std::flush;
        while (!quitting)
        {   
            std::chrono::time_point<std::chrono::system_clock> debut, fin;
            debut = std::chrono::system_clock::now();
            if (jours_écoulés%365 == 0)// Si le premier Octobre (début de l'année pour l'épidémie ;-) )
            {
                grippe = épidémie::Grippe(jours_écoulés/365);
                jour_apparition_grippe = grippe.dateCalculImportationGrippe();
                grippe.calculNouveauTauxTransmission();
                // 23% des gens sont immunisés. On prend les 23% premiers
                for ( int ipersonne = 0; ipersonne < nombre_immunisés_grippe; ++ipersonne)
                {
                    population[ipersonne].devientImmuniséGrippe();
                }
                for ( int ipersonne = nombre_immunisés_grippe; ipersonne < int(contexte.taux_population); ++ipersonne )
                {
                    population[ipersonne].redevientSensibleGrippe();
                }
            }
            if (jours_écoulés%365 == std::size_t(jour_apparition_grippe))
            {
                for (int ipersonne = nombre_immunisés_grippe; ipersonne < nombre_immunisés_grippe + 25; ++ipersonne )
                {
                    population[ipersonne].estContaminé(grippe);
                }
            }
            // Mise à jour des statistiques pour les cases de la grille :
            màjStatistique(grille, population);
            // On parcout la population pour voir qui est contaminé et qui ne l'est pas, d'abord pour la grippe puis pour l'agent pathogène
            std::size_t compteur_grippe = 0, compteur_agent = 0, mouru = 0;
            for ( auto& personne : population )
            {
                if (personne.testContaminationGrippe(grille, contexte.interactions, grippe, agent))
                {
                    compteur_grippe ++;
                    personne.estContaminé(grippe);
                }
                if (personne.testContaminationAgent(grille, agent))
                {
                    compteur_agent ++;
                    personne.estContaminé(agent);
                }
                // On vérifie si il n'y a pas de personne qui veillissent de veillesse et on génère une nouvelle personne si c'est le cas.
                if (personne.doitMourir())
                {
                    mouru++;
                    unsigned nouvelle_graine = jours_écoulés + personne.position().x*personne.position().y;
                    personne = épidémie::Individu(nouvelle_graine, contexte.espérance_de_vie, contexte.déplacement_maximal);
                    personne.setPosition(largeur_grille, hauteur_grille);
                }
                personne.veillirDUnJour();
                personne.seDéplace(grille);
            }


            //données à envoyer à l'autre processus
            auto const& statistiques = grille.getStatistiques();
            int s = statistiques.size();
            int flag = 0;
            //MPI_Request request[5];

            MPI_Iprobe(0, MPI_ANY_TAG, comm, &flag, &status);
            if(flag)
            {
                MPI_Recv(&go, 1, MPI_INT, 0, MPI_ANY_TAG, comm, &status);
                MPI_Send(&s, 1, MPI_INT, 0, 0, comm);
                MPI_Send(&(grille.dimension()[0]), 1, MPI_INT, 0, 1, comm);
                MPI_Send(&(grille.dimension()[1]), 1, MPI_INT, 0, 2, comm);
                MPI_Send(statistiques.data(), statistiques.size(), MPI_StatParCase, 0, 3, comm);
                MPI_Send(&jours_écoulés, 1, MPI_INT, 0, 4, comm);
                //MPI_Wait(&request[3], &status);
            }
            


            output << jours_écoulés << "\t" << grille.nombreTotalContaminésGrippe() << "\t"
               << grille.nombreTotalContaminésAgentPathogène() << std::endl;
            jours_écoulés += 1;
            
            fin = std::chrono::system_clock::now();
            std::chrono::duration<double> temps_sim = fin - debut;

            std::cout << "Temps simulation : " << temps_sim.count() << std::endl;
        }
        output.close();

    }
    
    
}


    


    //while (!quitting)
    //{
        //std::chrono::time_point<std::chrono::system_clock> debut, milieu, fin;
        //debut = std::chrono::system_clock::now();

        
        //milieu = std::chrono::system_clock::now();

        //#############################################################################################################
        //##    Affichage des résultats pour le temps  actuel
        //#############################################################################################################

        /*std::cout << jours_écoulés << "\t" << grille.nombreTotalContaminésGrippe() << "\t"
                  << grille.nombreTotalContaminésAgentPathogène() << std::endl;*/

        

        //fin = std::chrono::system_clock::now();

        /*
        std::chrono::duration<double> temps_tot = fin - debut;
        std::chrono::duration<double> temps_sim = milieu - debut;
        std::chrono::duration<double> temps_aff = fin - milieu;

        std::cout << "Temps simulation : " << temps_sim.count() << ", temps affichage =" <<
        temps_aff.count() << ", temps total :" << temps_tot.count() << std::endl;
        */
    //}// Fin boucle temporelle
//}

int main(int argc, char* argv[])
{
    // On initialise le contexte MPI
    MPI_Init(&argc, &argv);
    // on préfère toujours cloner le communicateur global
    // MPI_COMM_WORLD qui gère l'ensemble des processus lancés par MPI.
    MPI_Comm globComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
    // On interroge le communicateur global pour connaître le nombre de processus
    // qui ont été lancés par l'utilisateur :
    int nbp;
    MPI_Comm_size(globComm, &nbp);

    if (nbp!=2)
    {
        std::cout << "Ce programme fonctionne avec 2 processus exactement." << std::endl;
        return 1;
    }

    // On interroge le communicateur global pour connaître l'identifiant qui
    // m'a été attribué ( en tant que processus ). Cet identifiant est compris
    // entre 0 et nbp-1 ( nbp étant le nombre de processus qui ont été lancés par
    // l'utilisateur )
    int rank;
    MPI_Comm_rank(globComm, &rank);


    // parse command-line
    bool affiche = true;
    for (int i=0; i<argc; i++) {
      std::cout << i << " " << argv[i] << "\n";
      if (std::string("-nw") == argv[i]) affiche = false;
    }
  
    sdl2::init(argc, argv);
    {
        simulation(affiche, globComm);
    }
    sdl2::finalize();


    // A la fin du programme, on doit synchroniser une dernière fois tous les processus
    // afin qu'aucun processus ne se termine pendant que d'autres processus continuent.
    MPI_Finalize();
    return EXIT_SUCCESS;
}
